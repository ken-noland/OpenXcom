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
#include "../Filesystem/CompositeFileSystem.h"
#include "ModInfo.h"
#include <filesystem>
#include <memory>
#include <unordered_map>

namespace OpenXcom
{

class EngineContext;
class FolderEntry;

struct ScannedMod
{
	ModInfo info;
	std::filesystem::path path;

	// Once the mod has been loaded, this pointer will be invalid. Storing it here to cache the filesystem
	std::unique_ptr<CompositeFileSystem> filesystem;
};

class ModScanner
{
protected:
	EngineContext& _engine;

	// used to scan the available mods
	CompositeFileSystem _modFilesystem;

	// stored the mods and their individual filesystems
	std::vector<ScannedMod> _scannedMods;

public:
	ModScanner(EngineContext& context);
	~ModScanner();

	// add in a specific mod folder(eg: <data filesystem>/core/xcom1)
	bool addModFolder(const std::unique_ptr<FolderEntry>& folder);

	// add in a folder to scan for mods(eg: <user filesystem>/mods)
	bool addScanFolder(const std::unique_ptr<FolderEntry>& folder);

	// Scan the directories for mods containing a metadata file
	// Note: This also sets of the mod filesystems to be used by the mods themselves
	bool scan();

	// Get the scanned mods(note: this moves the internal list to the destination instead of copying)
	std::vector<ScannedMod> takeScannedMods() { return std::move(_scannedMods); };
};

} // namespace OpenXcom
