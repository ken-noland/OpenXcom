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
struct ImageLoadParams;

class ImagePNGFileProcessor
{
protected:
	EngineContext& _context;

public:
	ImagePNGFileProcessor(EngineContext& context);
	~ImagePNGFileProcessor();

	bool load(ImageFile& out, const std::string& name, const std::filesystem::path& filename, ImageLoadParams& params);

	bool save(const std::filesystem::path& filename, ImageFile& imageData);
};

} // namespace OpenXcom
