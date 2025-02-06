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
#include <filesystem>

namespace OpenXcom
{

class EngineContext;
class HostImage;
class Palette;
class ImageFile;

class ImageBMPFileProcessor
{
protected:
	EngineContext& _context;

public:
	ImageBMPFileProcessor(EngineContext& context);
	~ImageBMPFileProcessor();

	ImageFile load(const std::string& name, const std::filesystem::path& filename, bool loadPalette = false);
	ImageFile load(const std::string& name, const uint8_t* buffer, std::size_t size, bool loadPalette = false);

	bool save(const std::filesystem::path& filename, ImageFile& imageData);
};

} // namespace OpenXcom
