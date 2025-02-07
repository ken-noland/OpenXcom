#pragma once
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
#include "../Handle.h"

#include <string>
#include <array>
#include <unordered_map>
#include <vector>
#include <glm/ext/vector_int2.hpp>

struct hb_face_t;
struct hb_font_t;

namespace OpenXcom
{

using CodePoint = uint32_t;
class DeviceImage;

struct Glyph
{
	uint16_t x, y, width, height;
	int8_t xOffset, yOffset, xAdvance;
};

struct PositionedGlyph
{
	char32_t codepoint;
	glm::ivec2 position;
	int32_t advance;
	int32_t offsetX, offsetY; // Adjustments for script shaping
	CodePoint glyphID;
};

class Font
{
private:
	OwningHandle<DeviceImage> _fontTexture; // The font atlas
	hb_face_t* _hbFace = nullptr;
	hb_font_t* _hbFont = nullptr;

	std::array<Glyph, 128> _asciiGlyphs;                 // Fast lookup for Codepage 437
	std::unordered_map<char32_t, Glyph> _extendedGlyphs; // Fallback for Unicode extensions

	void initializeHarfBuzz();

public:
	Font(OwningHandle<DeviceImage> texture, const std::array<Glyph, 128>& asciiGlyphs, const std::unordered_map<char32_t, Glyph>& extendedGlyphs = {});
	~Font();

	const Glyph* getGlyph(char32_t codepoint) const;

	glm::ivec2 getTextExtents(const std::string& text) const;

	std::vector<PositionedGlyph> shapeText(const std::string& text, glm::ivec2 position) const;
	std::vector<PositionedGlyph> wrappedText(const std::string& text, glm::ivec2 position, int maxWidth) const;
};

}
