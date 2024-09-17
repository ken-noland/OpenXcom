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

namespace OpenXcom
{

/**
 * Maps canonical names to file paths and maintains the virtual file system
 * for resource files.
 */
namespace FileMap
{
struct FileRecord
{
	std::string fullpath; // includes zip file name if any

	void* zip;     // borrowed reference/weakref. NOTNULL:
	size_t findex; // file index in the zipfile.

	FileRecord();

// SDLHACK
	///// Open file warped in RWops.
	//SDL_RWops* getRWops() const;
	///// Read the whole file to memory and warp in RWops.
	//SDL_RWops* getRWopsReadAll() const;

	std::unique_ptr<std::istream> getIStream() const;
	YAML::Node getYAML() const;
	std::vector<YAML::Node> getAllYAML() const;
};

} // namespace FileMap

} // namespace OpenXcom
