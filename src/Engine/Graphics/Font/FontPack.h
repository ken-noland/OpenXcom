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
#include <filesystem>
#include <vector>

#include "../../Resource/Handle.h"

namespace OpenXcom
{

class EngineContext;


struct FileFontImageDefinition
{
	std::filesystem::path file;
	std::u32string chars;
	uint32_t width;
	int32_t spacing;
};

struct FileFontDefinition
{
	std::string id;
	uint32_t width;
	uint32_t height;
	int32_t spacing;
	std::vector<FileFontImageDefinition> images;
};

struct FileFont
{
	std::vector<FileFontDefinition> fonts;
};


// The classic OpenXcom stored their fonts in a font pack, which is a collection of fonts. Since we've moved to an owning handle solution, we need a wrapper to manage the fonts.
class FontPack
{
protected:
	EngineContext& _context;
	std::vector<OwningHandle<FontPack>> _fontPackHandles;

	void load(const std::filesystem::path& fontPackPath);

public:
	[[deprecated("Font packs are the old way of loading fonts. Explicit ownership of individual fonts is the preferred way. This feature will go away in future releases")]]
	FontPack(EngineContext& context, const std::filesystem::path& fontPackPath);
	~FontPack();
};

} // namespace OpenXcom
