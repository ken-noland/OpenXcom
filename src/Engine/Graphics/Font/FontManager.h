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
#include "../../Resource/ResourceManager.h"
#include "Font.h"

#include <filesystem>

namespace OpenXcom
{

class FontManager : public ResourceManager<Font>
{
public:
	FontManager();
	virtual ~FontManager();

	// load font from memory
	OwningHandle<Font> loadFont(const std::string& name, OwningHandle<DeviceImage> texture, const std::array<Glyph, 128>& asciiGlyphs, const std::unordered_map<char32_t, Glyph>& extendedGlyphs = {});

	// load font from file
	OwningHandle<Font> loadFont(const std::string& name, const std::filesystem::path& filename);

	// get a font handle by name
	ResourceHandle<Font> getFontByName(const std::string& name) const;
};

} // namespace OpenXcom
