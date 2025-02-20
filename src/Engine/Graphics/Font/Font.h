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
#include "../../Resource/Handle.h"
#include "../../Json.h"

#include <string>
#include <array>
#include <unordered_map>
#include <vector>
#include <glm/ext/vector_int2.hpp>

struct hb_face_t;
struct hb_font_t;
struct hb_buffer_t;

namespace OpenXcom
{

using CodePoint = uint32_t;
class DeviceImage;
class DeviceBuffer;

struct Glyph
{
	int16_t x = -1, y = -1, width = -1, height = -1;
	int8_t xOffset = -1, yOffset = -1, xAdvance = -1;
	ResourceHandle<DeviceImage> image;
};

struct PositionedGlyph
{
	char32_t codepoint;
	glm::ivec2 position;
	int32_t advance;
	int32_t offsetX, offsetY; // Adjustments for script shaping
	CodePoint glyphID;
};

struct FontSettings
{
	// Default width
	uint8_t width;

	// Default height
	uint8_t height;

	// Default spacing
	uint8_t spacing;

	// The default palette index for the font. This is used when rendering the font with a palette
	uint8_t defaultPaletteIndex;

	// Some fonts, like the DosFont, only require a single palette entry. Other fonts, like the in-game
	// fonts, use multiple palette entries. This is the number of palette entries used by a glyph in the
	// font. It allows us to calculate the correct palette index for a given glyph.
	uint8_t numPaletteEntries;
};

class Font
{
protected:
	const std::string _name;

	FontSettings _settings;

	std::vector<OwningHandle<DeviceImage>> _fontTextures; // The font atlas

	std::array<Glyph, 128> _asciiGlyphs;                  // Fast lookup for Codepage 437
	std::unordered_map<char32_t, Glyph> _extendedGlyphs;  // Fallback for Unicode extensions

	// HarfBuzz font and face
	hb_face_t* _hbFace = nullptr;
	hb_font_t* _hbFont = nullptr;

	// Temporary buffer for HarfBuzz text shaping
	mutable hb_buffer_t* _tempBuffer = nullptr;

	void initializeHarfBuzz();

	// allow serialization to access the protected members
	friend bool fromJson<>(const nlohmann::json& json, Font& font);
	friend bool toJson<>(const Font& font, nlohmann::json& json);

public:
	Font(const std::string& name, const FontSettings& settings, OwningHandle<DeviceImage> texture, const std::array<Glyph, 128>& asciiGlyphs, const std::unordered_map<char32_t, Glyph>& extendedGlyphs = {});
	Font(const std::string& name, const FontSettings& settings, std::vector<OwningHandle<DeviceImage>> textures, const std::array<Glyph, 128>& asciiGlyphs, const std::unordered_map<char32_t, Glyph>& extendedGlyphs = {});
	~Font();

	const std::string& getName() const { return _name; }

	const Glyph* getGlyph(char32_t codepoint) const;

	const FontSettings& getSettings() const { return _settings; }

	void setLineSpacing(uint32_t lineSpacing) { _settings.spacing = lineSpacing; }
	uint32_t getLineSpacing() const { return _settings.spacing; }

	uint8_t getDefaultPaletteIndex() const { return _settings.defaultPaletteIndex; }
	uint8_t getNumPaletteEntries() const { return _settings.numPaletteEntries; }

	glm::ivec2 getTextExtents(const std::string& text) const;
	glm::ivec2 getTextExtents(const std::u32string& text) const;

	glm::ivec2 getTextExtents(const std::string_view& text) const;
	glm::ivec2 getTextExtents(const std::u32string_view& text) const;

	std::vector<PositionedGlyph> shapeText(const std::string_view& text, glm::ivec2 position) const;
	std::vector<PositionedGlyph> shapeText(const std::u32string_view& text, glm::ivec2 position) const;
};

} // namespace OpenXcom
