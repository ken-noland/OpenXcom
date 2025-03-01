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
#include "ModScanner.h"
#include "../Logger.h"
#include "../YamlFile.h"

namespace OpenXcom
{

ModScanner::ModScanner(EngineContext& context)
	: _engine(context)
{
}

ModScanner::~ModScanner()
{
}

bool ModScanner::addModFolder(const std::unique_ptr<FolderEntry>& folder)
{
	if (!folder)
	{
		Log(LOG_WARNING) << "Mod folder is null";
		return false;
	}

	//validate the folder is a folder and that it exists
	std::filesystem::path path = folder->getPath();

	if (!std::filesystem::is_directory(path))
	{
		Log(LOG_WARNING) << "Mod folder '" << path << "' does not exist or is not a directory";
		return false;
	}

	std::unique_ptr<FileEntry> metadataFile = folder->getFile("metadata.yml");

	ScannedMod mod;

	// load up the metadata file so we have it for future reference
	YamlFile metadataYmlFile;
	mod.info = metadataYmlFile.load<ModInfo>(metadataFile);

	mod.path = path;
	mod.filesystem = std::make_unique<CompositeFileSystem>();
	mod.filesystem->addFileSystem(folder->createFileSystem());

	_scannedMods.push_back(std::move(mod));

	return true;
}

bool ModScanner::addScanFolder(const std::unique_ptr<FolderEntry>& folder)
{
	if (!folder)
	{
		Log(LOG_WARNING) << "Mod folder is null";
		return false;
	}

	_modFilesystem.addFileSystem(folder->createFileSystem());
	return true;
}

bool ModScanner::scan()
{
	// the mod filesystem contains the directories where mods are currently present.
	for (FileSystemIterator it = _modFilesystem.begin(); it != _modFilesystem.end(); ++it)
	{
		std::unique_ptr<VFSEntry> entry = (*it);

		// if it's a directory, check if it's a mod
		if (entry->getType() == VFSEntry::EntryType::Directory)
		{
			// convert to FolderEntry
			std::unique_ptr<FolderEntry> folderPtr(static_cast<FolderEntry*>(entry.release()));

			// check if it has a metadata.yml file
			std::unique_ptr<FileEntry> metadataFile = folderPtr->getFile("metadata.yml");
			if (!metadataFile)
			{
				// TODO: add support for json metadata files
				continue;
			}

			// add it to our list of mods
			addModFolder(folderPtr);
		}
		// TODO: if it's a file, then we need to check if it's a zip file and then add it if it contains a metadata.yml

	}

	return true;
}

} // namespace OpenXcom
