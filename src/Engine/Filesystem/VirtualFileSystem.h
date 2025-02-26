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
 *e
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "CompositeFilesystem.h"
#include "EmbeddedFileSystem.h"
#include "PhysicalFileSystem.h"
#include "ZipFileSystem.h"

#include <filesystem>
#include <map>
#include <memory>
#include <vector>

namespace OpenXcom
{

class FileSystem;
class Options;

// The virtual filesystem manages all other files systems
//
// The virtual filesystem is a way to abstract the filesystem so that the
// game can access files in a consistent way, regardless of where the files
// are stored. This allows the game to access files that are embedded in
// the executable, stored on the physical filesystem, or stored in a zip
// archive, without having to know where the files are stored.
//
// During initialization, it takes the paths provided for "data", "user",
// and "config" and creates the appropriate filesystems for each of them.
// For data, it creates a CompositeFilesystem that contains an
// EmbeddedFileSystem, (potentially) a few PhysicalFilesystems, and (potentially)
// a few ZipFilesystems. For user and config, it creates a PhysicalFileSystem.
// Data contains resources such as "common", "standard", as well as the games
// base data files for Xcom.
//
// There's also a special case here because "standard" is actually a list of
// mods. Special handling is done to allow "standard" to be included in the
// "mods" special composite filesystem. Because mods can be located in the
// user directory, either as folders or as zips, it also needs to be a special
// composite filesystem.
//
// For mods, they can only access files that are within their mod path, so the
// filesystem is locked to that path. This is to prevent mods from accessing
// files outside of their mod directory.
class VirtualFileSystem
{
public:
	VirtualFileSystem(const Options& options);
	~VirtualFileSystem();

	FileSystem& getDataFileSystem();
	FileSystem& getModFileSystem();

protected:
	// there can only be one embedded filesystem
	EmbeddedFileSystem _embedded;

	// data and mods are composite filesystems, meaning they can contain
	// multiple filesystems(embedded, physical and zip filesystems)
	CompositeFilesystem _data;
	CompositeFilesystem _mods;
};

} // namespace OpenXcom
