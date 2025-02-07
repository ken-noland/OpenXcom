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

#include "../../Graphics/Image/Image.h"
#include "../../Graphics/Image/ImageManager.h"

#include <hb.h>
#include <linebreak.h>

namespace OpenXcom
{

Font::Font(OwningHandle<DeviceImage> texture, const std::array<Glyph, 128>& asciiGlyphs, const std::unordered_map<char32_t, Glyph>& extendedGlyphs)
	: _fontTexture(std::move(texture)), _asciiGlyphs(asciiGlyphs), _extendedGlyphs(extendedGlyphs)
{

	// Initialize HarfBuzz with a dummy face (no TrueType tables needed)
	_hbFace = hb_face_create_for_tables([](hb_face_t* face, hb_tag_t tag, void* user_data) -> hb_blob_t* {
		return nullptr;
	},
										nullptr, nullptr);
	_hbFont = hb_font_create(_hbFace);

	initializeHarfBuzz();
}

Font::~Font()
{
	hb_font_destroy(_hbFont);
	hb_face_destroy(_hbFace);
}

void Font::initializeHarfBuzz()
{
	hb_font_funcs_t* funcs = hb_font_funcs_create();

	// Override HarfBuzz's glyph retrieval with our bitmap font data
	hb_font_funcs_set_nominal_glyph_func(funcs, [](hb_font_t*, void*, hb_codepoint_t unicode, hb_codepoint_t* glyph, void* userData) -> hb_bool_t {
		if (unicode < 128)
		{
			*glyph = unicode; // Direct ASCII mapping
			return true;
		}
		return false; // No fallback handling for now
	},
										 nullptr, nullptr);

	// Override advance width function
	hb_font_funcs_set_glyph_h_advance_func(funcs, [](hb_font_t*, void* fontData, hb_codepoint_t glyph, void* userData) -> hb_position_t {
		Font* font = static_cast<Font*>(userData);
		const Glyph* g = font->getGlyph(glyph);
		return g ? g->xAdvance * 64 : 9 * 64; // Default to 9 pixels advance
	},
										   this, nullptr);

	hb_font_set_funcs(_hbFont, funcs, this, nullptr);
	hb_font_set_scale(_hbFont, 9 * 64, 16 * 64); // Fixed 26.6 format
}

const Glyph* Font::getGlyph(char32_t codepoint) const
{
	if (codepoint < 128) return &_asciiGlyphs[codepoint];
	auto it = _extendedGlyphs.find(codepoint);
	return (it != _extendedGlyphs.end()) ? &it->second : nullptr;
}

glm::ivec2 Font::getTextExtents(const std::string& text) const
{
	// Shape the text at (0,0) to obtain positioned glyphs.
	std::vector<PositionedGlyph> glyphs = shapeText(text, glm::ivec2(0, 0));

	if (glyphs.empty())
	{
		return glm::ivec2(0, 0);
	}

	// One way: The total width is the x position of the last glyph plus its advance.
	// (This assumes your shaping function positions the first glyph at x = 0.)
	int32_t width = glyphs.back().position.x + glyphs.back().advance;

	return glm::ivec2(width, 16); // Fixed font height
}

// Generates positioned glyphs for rendering
std::vector<PositionedGlyph> Font::shapeText(const std::string& text, glm::ivec2 position) const
{
	std::vector<PositionedGlyph> positionedGlyphs;

	hb_buffer_t* buffer = hb_buffer_create();
	int size = static_cast<int>(text.size());
	hb_buffer_add_utf8(buffer, text.c_str(), size, 0, size);
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

std::vector<PositionedGlyph> Font::wrappedText(const std::string& text, glm::ivec2 position, int maxWidth) const
{
	std::vector<PositionedGlyph> allGlyphs;
	size_t textLen = text.size();

	// Allocate a buffer for break properties for each byte of the UTF-8 text.
	std::vector<char> breakProps(textLen);
	// Use libunibreak (via libuniwrapper) to fill breakProps.
	set_linebreaks_utf8(reinterpret_cast<const utf8_t*>(text.c_str()), textLen, "", breakProps.data());

	// Helper lambda to measure the width of a given substring.
	auto measureTextWidth = [this](const std::string& s) -> float {
		// Shape the text at (0,0); we only need the advances.
		auto glyphs = this->shapeText(s, glm::ivec2(0, 0));
		float width = 0.0f;
		for (const auto& g : glyphs)
			width += g.advance;
		return width;
	};

	size_t start = 0;
	int lineSpacing = 2; // Extra pixels between lines.
	int currentY = position.y;

	// Process the entire string.
	while (start < textLen)
	{
		size_t bestBreak = start;
		size_t pos = start;
		// Greedily extend the candidate until it no longer fits.
		while (pos < textLen)
		{
			// If this position is a valid break point, consider it.
			if (breakProps[pos] == LINEBREAK_ALLOWBREAK ||
				breakProps[pos] == LINEBREAK_MUSTBREAK)
			{
				// Candidate substring from 'start' up through this character.
				std::string candidate = text.substr(start, pos - start + 1);
				float candidateWidth = measureTextWidth(candidate);
				if (candidateWidth <= maxWidth)
				{
					bestBreak = pos + 1; // Break after this character.
				}
				else
				{
					break; // Exceeded maxWidth.
				}
			}
			pos++;
		}
		// If no break was found that advances the pointer, force a break at one character.
		if (bestBreak == start)
		{
			bestBreak = start + 1;
		}

		// Extract the line text.
		std::string lineStr = text.substr(start, bestBreak - start);
		// Shape the line (positioned at the desired x and current y).
		auto lineGlyphs = this->shapeText(lineStr, glm::ivec2(position.x, currentY));
		// Append the glyphs for this line.
		allGlyphs.insert(allGlyphs.end(), lineGlyphs.begin(), lineGlyphs.end());

		// Advance start to the next segment.
		start = bestBreak;
		// Move currentY down by the font's height (here, 16 pixels) plus line spacing.
		currentY += 16 + lineSpacing;
	}
	return allGlyphs;
}


} // namespace OpenXcom
