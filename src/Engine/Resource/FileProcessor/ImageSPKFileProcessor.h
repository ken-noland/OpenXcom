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

#include <glm/vec2.hpp>

namespace OpenXcom
{

class EngineContext;
class FileEntry;
struct ImageFile;

struct ImageSPKLoadParams
{
	glm::ivec2 extent;
};

class ImageSPKFileProcessor
{
protected:
	EngineContext& _context;

public:
	ImageSPKFileProcessor(EngineContext& context);
	~ImageSPKFileProcessor();

	ImageFile load(const std::string& name, const std::unique_ptr<FileEntry>& file, const ImageSPKLoadParams& params);
};

} // namespace OpenXcom
