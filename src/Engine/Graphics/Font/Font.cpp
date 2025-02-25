/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "Font.h"

#include "../Image/Image.h"
#include "../Image/ImageManager.h"
#include "../Buffer/Buffer.h"

#include <hb.h>
#include <linebreak.h>

#include "../../Utility/RTTR.h"

namespace OpenXcom
{


SIMPLERTTR
{
	// Font class will be handled independently

	// Glyph class will be handled independently

	SimpleRTTR::registration().type<FontSettings>()
		.meta("Serialize", ObjectSerialize::ALWAYS) // always serialize this
		.property(REGISTER_PROPERTY(FontSettings, width))
			.meta("Serialize", PropertySerialize::ALWAYS)
		.property(REGISTER_PROPERTY(FontSettings, height))
			.meta("Serialize", PropertySerialize::ALWAYS)
		.property(REGISTER_PROPERTY(FontSettings, spacing))
			.meta("Serialize", PropertySerialize::ALWAYS)
		.property(REGISTER_PROPERTY(FontSettings, defaultPaletteIndex))
			.meta("Serialize", PropertySerialize::ALWAYS)
		.property(REGISTER_PROPERTY(FontSettings, numPaletteEntries))
			.meta("Serialize", PropertySerialize::ALWAYS);
}

Font::Font(const std::string& name, const FontSettings& settings, OwningHandle<DeviceImage> texture, const std::array<Glyph, 128>& asciiGlyphs, const std::unordered_map<char32_t, Glyph>& extendedGlyphs)
	: Font(name, settings, [&texture]() {
          std::vector<OwningHandle<DeviceImage>> textures;
          textures.reserve(1);
          textures.emplace_back(std::move(texture));
          return textures; }(), asciiGlyphs, extendedGlyphs)
{
}

Font::Font(const std::string& name, const FontSettings& settings, std::vector<OwningHandle<DeviceImage>> textures, const std::array<Glyph, 128>& asciiGlyphs, const std::unordered_map<char32_t, Glyph>& extendedGlyphs)
	: _name(name), _settings(settings), _fontTextures(std::move(textures)), _asciiGlyphs(asciiGlyphs), _extendedGlyphs(extendedGlyphs)
{
	// Initialize HarfBuzz with a dummy face (no TrueType tables needed)
	_hbFace = hb_face_create_for_tables([](hb_face_t* face, hb_tag_t tag, void* user_data) -> hb_blob_t* {
		return nullptr;
	},
										nullptr, nullptr);
	_hbFont = hb_font_create(_hbFace);

	_tempBuffer = hb_buffer_create();

	initializeHarfBuzz();
}

Font::~Font()
{
	if (_tempBuffer)
	{
		hb_buffer_destroy(_tempBuffer);
		_tempBuffer = nullptr;
	}

	hb_font_funcs_destroy(_funcs);
	hb_font_destroy(_hbFont);
	hb_face_destroy(_hbFace);
}

void Font::initializeHarfBuzz()
{
	_funcs = hb_font_funcs_create();

	// Override HarfBuzz's glyph retrieval with our bitmap font data
	hb_font_funcs_set_nominal_glyph_func(_funcs, [](hb_font_t*, void*, hb_codepoint_t unicode, hb_codepoint_t* glyph, void* userData) -> hb_bool_t {
		if (unicode < 128)
		{
			*glyph = unicode; // Direct ASCII mapping
			return true;
		}
		return false; // No fallback handling for now
	}, nullptr, nullptr);

	// Override advance width function
	hb_font_funcs_set_glyph_h_advance_func(_funcs, [](hb_font_t*, void* fontData, hb_codepoint_t glyph, void* userData) -> hb_position_t {
		Font* font = static_cast<Font*>(userData);
		const Glyph* g = font->getGlyph(glyph);
		return g ? g->xAdvance * 64 : 9 * 64; // Default to 9 pixels advance
	}, this, nullptr);

	hb_font_set_funcs(_hbFont, _funcs, this, nullptr);
	//hb_font_set_scale(_hbFont, 9 * 64, 16 * 64); // Fixed 26.6 format
}

const Glyph* Font::getGlyph(char32_t codepoint) const
{
	if (codepoint < 128) return &_asciiGlyphs[codepoint];
	auto it = _extendedGlyphs.find(codepoint);
	return (it != _extendedGlyphs.end()) ? &it->second : nullptr;
}

glm::ivec2 Font::getTextExtents(const std::string& text) const
{
	return getTextExtents(std::string_view(text));
}

glm::ivec2 Font::getTextExtents(const std::u32string& text) const
{
	return getTextExtents(std::u32string_view(text));
}

glm::ivec2 Font::getTextExtents(const std::string_view& text) const
{
	// Create a HarfBuzz buffer and add the UTF-8 text.
	hb_buffer_clear_contents(_tempBuffer);

	int size = static_cast<int>(text.size());
	hb_buffer_add_utf8(_tempBuffer, text.data(), size, 0, size);
	hb_buffer_guess_segment_properties(_tempBuffer);

	// Shape the text using our HarfBuzz font.
	hb_shape(_hbFont, _tempBuffer, nullptr, 0);

	// Retrieve the glyph positions.
	unsigned int glyphCount = 0;
	hb_glyph_position_t* glyphPositions = hb_buffer_get_glyph_positions(_tempBuffer, &glyphCount);

	int totalAdvance = 0;
	for (unsigned int i = 0; i < glyphCount; i++)
	{
		totalAdvance += glyphPositions[i].x_advance; // x_advance is in 26.6 fixed-point format.
	}

	// Convert total advance from fixed-point (26.6) to integer pixels.
	int width = totalAdvance / 64;

	// For this bitmap font, the height is constant (e.g., 16 pixels).
	return glm::ivec2(width, _settings.height);
}

glm::ivec2 Font::getTextExtents(const std::u32string_view& text) const
{
	// Create a HarfBuzz buffer and add the UTF-8 text.
	hb_buffer_clear_contents(_tempBuffer);

	int size = static_cast<int>(text.size());
	hb_buffer_add_utf32(_tempBuffer, reinterpret_cast<const uint32_t*>(text.data()), size, 0, size);
	hb_buffer_guess_segment_properties(_tempBuffer);

	// Shape the text using our HarfBuzz font.
	hb_shape(_hbFont, _tempBuffer, nullptr, 0);

	// Retrieve the glyph positions.
	unsigned int glyphCount = 0;
	hb_glyph_position_t* glyphPositions = hb_buffer_get_glyph_positions(_tempBuffer, &glyphCount);

	int totalAdvance = 0;
	for (unsigned int i = 0; i < glyphCount; i++)
	{
		totalAdvance += glyphPositions[i].x_advance; // x_advance is in 26.6 fixed-point format.
	}

	// Convert total advance from fixed-point (26.6) to integer pixels.
	int width = totalAdvance / 64;

	// For this bitmap font, the height is constant (e.g., 16 pixels).
	return glm::ivec2(width, _settings.height);
}

// Generates positioned glyphs for rendering
std::vector<PositionedGlyph> Font::shapeText(const std::string_view& text, glm::ivec2 position) const
{
	std::vector<PositionedGlyph> positionedGlyphs;

	hb_buffer_t* buffer = hb_buffer_create();
	int size = static_cast<int>(text.size());
	hb_buffer_add_utf8(buffer, text.data(), size, 0, size);
	hb_buffer_guess_segment_properties(buffer);

	hb_shape(_hbFont, buffer, nullptr, 0);

	unsigned int glyphCount;
	hb_glyph_info_t* glyphInfo = hb_buffer_get_glyph_infos(buffer, &glyphCount);
	hb_glyph_position_t* glyphPos = hb_buffer_get_glyph_positions(buffer, &glyphCount);

	float x = static_cast<float>(position.x);
	float y = static_cast<float>(position.y);

	for (unsigned int i = 0; i < glyphCount; i++)
	{
		char32_t codepoint = glyphInfo[i].codepoint;
		const Glyph* glyph = getGlyph(codepoint);
		if (!glyph) continue; // Skip missing characters

		glm::ivec2 pos = {static_cast<int>(x + glyphPos[i].x_offset / 64.0f), // Apply HarfBuzz offset (fixed 26.6 format)
						  static_cast<int>(y + glyphPos[i].y_offset / 64.0f)};

		positionedGlyphs.push_back({codepoint,
									pos,
									static_cast<int32_t>(glyphPos[i].x_advance / 64.0f),
									static_cast<int32_t>(glyphPos[i].x_offset / 64.0f),
									static_cast<int32_t>(glyphPos[i].y_offset / 64.0f),
									glyphInfo[i].codepoint});

		x += glyphPos[i].x_advance / 64.0f; // Move cursor forward
	}

	hb_buffer_destroy(buffer);
	return positionedGlyphs;
}


// Generates positioned glyphs for rendering
std::vector<PositionedGlyph> Font::shapeText(const std::u32string_view& text, glm::ivec2 position) const
{
	std::vector<PositionedGlyph> positionedGlyphs;

	hb_buffer_t* buffer = hb_buffer_create();
	int size = static_cast<int>(text.size());
	hb_buffer_add_utf32(buffer, reinterpret_cast<const uint32_t*>(text.data()), size, 0, size);
	hb_buffer_guess_segment_properties(buffer);

	hb_shape(_hbFont, buffer, nullptr, 0);

	unsigned int glyphCount;
	hb_glyph_info_t* glyphInfo = hb_buffer_get_glyph_infos(buffer, &glyphCount);
	hb_glyph_position_t* glyphPos = hb_buffer_get_glyph_positions(buffer, &glyphCount);

	float x = static_cast<float>(position.x);
	float y = static_cast<float>(position.y);

	for (unsigned int i = 0; i < glyphCount; i++)
	{
		char32_t codepoint = glyphInfo[i].codepoint;
		const Glyph* glyph = getGlyph(codepoint);
		if (!glyph) continue; // Skip missing characters

		glm::ivec2 pos = {static_cast<int>(x + glyphPos[i].x_offset / 64.0f), // Apply HarfBuzz offset (fixed 26.6 format)
						  static_cast<int>(y + glyphPos[i].y_offset / 64.0f)};

		positionedGlyphs.push_back({codepoint,
									pos,
									static_cast<int32_t>(glyphPos[i].x_advance / 64.0f),
									static_cast<int32_t>(glyphPos[i].x_offset / 64.0f),
									static_cast<int32_t>(glyphPos[i].y_offset / 64.0f),
									glyphInfo[i].codepoint});

		x += glyphPos[i].x_advance / 64.0f; // Move cursor forward
	}

	hb_buffer_destroy(buffer);
	return positionedGlyphs;
}

} // namespace OpenXcom
