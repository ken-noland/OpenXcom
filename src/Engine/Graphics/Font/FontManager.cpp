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

#include "FontManager.h"
#include "../Image/Image.h"

#include "../../EngineContext.h"
#include "../../Logger.h"
#include "../../Filesystem/VirtualFileSystem.h"

namespace OpenXcom
{

FontManager::FontManager(EngineContext& context)
	: _context(context)
{
}

FontManager::~FontManager()
{
}

OwningHandle<Font> FontManager::load(const std::string& name, const FontSettings& settings, std::vector<OwningHandle<DeviceImage>> textures, const std::array<Glyph, 128>& asciiGlyphs, const std::unordered_map<char32_t, Glyph>& extendedGlyphs)
{
	return add(std::make_unique<Font>(name, settings, std::move(textures), asciiGlyphs, extendedGlyphs));
}

OwningHandle<Font> FontManager::load(const std::string& name, const std::filesystem::path& filename)
{
	// use the virtual file system to find the font file
	FileSystem& vfs = _context.getVirtualFileSystem().getDataFileSystem();
	std::unique_ptr<FileEntry> file = vfs.getFile(filename);

	if (!file)
	{
		Log(LOG_ERROR) << "FontManager: Could not find font file: " << filename;
		return OwningHandle<Font>();
	}


	throw std::runtime_error("FontManager::load not implemented");
	return OwningHandle<Font>();
}


ResourceHandle<Font> FontManager::getFontByName(const std::string& name) const
{
	for (const std::pair<const ResourceHandle<Font>, std::unique_ptr<Font>>& font : _resources)
	{
		if (font.second->getName() == name)
		{
			return font.first;
		}
	}
	return ResourceHandle<Font>::Invalid_Handle;
}

} // namespace OpenXcom
