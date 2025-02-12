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

#include <hb.h>
#include <linebreak.h>

namespace OpenXcom
{

Font::Font(const std::string& name, OwningHandle<DeviceImage> texture, const std::array<Glyph, 128>& asciiGlyphs, const std::unordered_map<char32_t, Glyph>& extendedGlyphs)
	: _name(name), _fontTexture(std::move(texture)), _asciiGlyphs(asciiGlyphs), _extendedGlyphs(extendedGlyphs)
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
	}, nullptr, nullptr);

	// Override advance width function
	hb_font_funcs_set_glyph_h_advance_func(funcs, [](hb_font_t*, void* fontData, hb_codepoint_t glyph, void* userData) -> hb_position_t {
		Font* font = static_cast<Font*>(userData);
		const Glyph* g = font->getGlyph(glyph);
		return g ? g->xAdvance * 64 : 9 * 64; // Default to 9 pixels advance
	}, this, nullptr);

	hb_font_set_funcs(_hbFont, funcs, this, nullptr);
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
	return glm::ivec2(width, 16);
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
	set_linebreaks_utf8(reinterpret_cast<const utf8_t*>(text.c_str()), textLen, "", breakProps.data());

	size_t start = 0;
	int currentY = position.y;
	const int lineHeight = 16;            // Fixed height of the bitmap font.
	const int lineSpacing = _lineSpacing; // Assume _lineSpacing is defined in Font.

	while (start < textLen)
	{
		size_t bestBreak = start;
		size_t lastAllowBreak = start; // Track the last index where LINEBREAK_ALLOWBREAK was seen.
		bool encounteredMustBreak = false;
		size_t pos = start;

		// Extend candidate from 'start' until either the candidate exceeds maxWidth
		// or we run out of text.
		while (pos < textLen)
		{
			char flag = breakProps[pos];

			// If a mandatory break is encountered, set bestBreak and exit immediately.
			if (flag == LINEBREAK_MUSTBREAK)
			{
				bestBreak = pos + 1;
				encounteredMustBreak = true;
				break;
			}

			// Record a candidate break if allowed.
			if (flag == LINEBREAK_ALLOWBREAK)
			{
				lastAllowBreak = pos + 1;
			}

			// Create candidate substring from 'start' to pos (inclusive).
			std::string candidate = text.substr(start, pos - start + 1);
			int32_t candidateWidth = getTextExtents(candidate).x;

			// If candidate fits within maxWidth, update bestBreak.
			if (candidateWidth <= maxWidth)
			{
				bestBreak = pos + 1;
			}
			else
			{
				// Candidate too wide: break out of the loop.
				break;
			}
			pos++;
		}

		// If we reached the end of text and the candidate still fits, take the rest.
		if (pos >= textLen)
		{
			std::string candidate = text.substr(start, textLen - start);
			if (getTextExtents(candidate).x <= maxWidth)
			{
				bestBreak = textLen;
			}
			else if (bestBreak == start && lastAllowBreak > start)
			{
				// If the very first candidate exceeded maxWidth, but we had an allowed break earlier,
				// use that instead.
				bestBreak = lastAllowBreak;
			}
		}

		// If no progress was made and we haven't encountered a MUSTBREAK,
		// force a break after one character.
		if (bestBreak == start)
		{
			bestBreak = start + 1;
		}

		// If we haven't encountered a MUSTBREAK and we have a valid allowed break,
		// prefer that if the candidate with it still fits.
		else if (!encounteredMustBreak && lastAllowBreak > start)
		{
			std::string candidate = text.substr(start, lastAllowBreak - start);
			if (getTextExtents(candidate).x <= maxWidth)
			{
				bestBreak = lastAllowBreak;
			}
		}

		// Extract the line text.
		std::string lineStr = text.substr(start, bestBreak - start);
		// Shape the line at the desired starting x and current y.
		std::vector<PositionedGlyph> lineGlyphs = this->shapeText(lineStr, glm::ivec2(position.x, currentY));
		allGlyphs.insert(allGlyphs.end(), lineGlyphs.begin(), lineGlyphs.end());

		start = bestBreak;
		currentY += lineHeight + lineSpacing;
	}
	return allGlyphs;
}


} // namespace OpenXcom
