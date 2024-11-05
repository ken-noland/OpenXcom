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
#include "../ResourceManager.h"
#include "Palette.h"

#include <filesystem>
#include <glm/vec4.hpp>

namespace OpenXcom
{

class PaletteManager : public ResourceManager<Palette>
{
public:
	PaletteManager();
	virtual ~PaletteManager();

	// load palette from memory
	Handle loadPalette(const std::string& name, const glm::ivec4* data, size_t size);

	// load palette from parameters
	Handle loadPalette(const std::string& name, std::initializer_list<glm::ivec4> data);

	// load palette from file
	Handle loadPalette(const std::string& name, const std::filesystem::path& path);
};

} // namespace OpenXcom
