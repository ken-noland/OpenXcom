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
#include "GameMods.h"
#include "GameContext.h"
#include "../Engine/EngineContext.h"
#include "../Engine/Options.h"
#include "../Engine/Filesystem/VirtualFileSystem.h"

namespace OpenXcom
{

GameMods::GameMods(GameContext& context)
	: _context(context)
{
}

GameMods::~GameMods()
{
}

bool GameMods::load()
{
	ModScanner scanner(_context.getEngineContext());

	// register all the required and optional game directories in the mod scanner
	if (!setupScanner(scanner)) { return false; }

	// scan all the directories
	if (!scanner.scan()) { return false; }
	_inactiveMods = scanner.takeScannedMods();

	// Okay, we now have a list of mods to work with. Now we need to filter them.

	// Step 1: move the masters to their own list
	for (std::vector<ScannedMod>::iterator it = _inactiveMods.begin(); it != _inactiveMods.end();)
	{
		if (it->info.type == ModType::Master)
		{
			_masters.push_back(std::move(*it));
			it = _inactiveMods.erase(it);
		}
		else
		{
			++it;
		}
	}

	// Step 2: move active mods to their own list
	std::vector<std::string> configActiveMods = _context.getEngineContext().getOptions().get<&GameOptions::_mods>();
	for (std::vector<ScannedMod>::iterator it = _inactiveMods.begin(); it != _inactiveMods.end();)
	{
		// find mod in config list
		std::vector<std::string>::iterator configIt = std::find(configActiveMods.begin(), configActiveMods.end(), it->info.id);
		if (configIt != configActiveMods.end())
		{
			_activeMods.push_back(std::move(*it));
			// remove it from the config list so we don't search for it again.
			configActiveMods.erase(configIt);
			it = _inactiveMods.erase(it);
		}
		else
		{
			++it;
		}
	}
	for (const std::string& modId : configActiveMods)
	{
		Log(LOG_WARNING) << "Mod " << modId << " is in the active mods list but was not found in the scanned mods list. Could not load the mod";
	}

	// Step 3: verify that all active mods have their dependencies met
	for (std::vector<ScannedMod>::iterator it = _activeMods.begin(); it != _activeMods.end();)
	{
		// TODO: have a separate function for this. One that could be used in game when trying to activate mods using the UI as well.
		bool dependenciesMet = true;
		for (DependencyExpression& dep : it->info.dependencies)
		{
			// check if the mod is loaded
			std::vector<ScannedMod>::iterator depIt = std::find_if(_activeMods.begin(), _activeMods.end(), [&dep](const ScannedMod& mod) { return mod.info.id == dep.mod; });
			if (depIt == _activeMods.end())
			{
				// check if the mod is a master
				depIt = std::find_if(_masters.begin(), _masters.end(), [&dep](const ScannedMod& mod) { return mod.info.id == dep.mod; });
				if (depIt == _masters.end())
				{
					// dependency not met
					dependenciesMet = false;
					break;
				}
			}
		}
		if (!dependenciesMet)
		{
			// re-add the mod to the scanned mods list
			_inactiveMods.push_back(std::move(*it));

			// remove the mod from the active list
			it = _activeMods.erase(it);
		}
		else
		{
			++it;
		}
	}

	return true;
}

bool GameMods::setupScanner(ModScanner& scanner)
{
	EngineContext& engineContext = _context.getEngineContext();
	VirtualFileSystem& vfs = engineContext.getVirtualFileSystem();

	FileSystem& dataFs = vfs.getDataFileSystem();
	FileSystem& userFs = vfs.getUserFileSystem();

	// xcom1 and xcom2 are hardcoded to exist in the <data filesystem>/core/xcom1
	// and <data filesystem>/core/xcom2 directories
	std::unique_ptr<FolderEntry> xcom1Path = dataFs.getFolder("core/xcom1");
	if (!xcom1Path)
	{
		Log(LOG_WARNING) << "Could not find core/xcom1 directory in data filesystem";
	}
	else
	{
		scanner.addModFolder(xcom1Path);
	}
	std::unique_ptr<FolderEntry> xcom2Path = dataFs.getFolder("core/xcom2");
	if (!xcom2Path)
	{
		Log(LOG_WARNING) << "Could not find core/xcom2 directory in data filesystem";
	}
	else
	{
		scanner.addModFolder(xcom2Path);
	}

	// there's only one user directory, so just add it directly
	std::unique_ptr<FolderEntry> userModsPath = userFs.getFolder("mods");
	if(userModsPath)
	{
		scanner.addScanFolder(userModsPath);
	}

	// but there are multiple possible data directories, so go through each one and add them individually
	for (FileSystemIterator it = dataFs.begin(); it != dataFs.end(); ++it)
	{
		std::unique_ptr<VFSEntry> entry = (*it);

		// if it's a directory, check if it's a mod
		if (entry->getType() == VFSEntry::EntryType::Directory)
		{
			// convert to FolderEntry
			std::unique_ptr<FolderEntry> folderPtr(static_cast<FolderEntry*>(entry.release()));

			// check the directory name to see if it is "mod"
			if (folderPtr->getPath().filename() == "mods")
			{
				scanner.addScanFolder(folderPtr);
			}
		}

		// TODO: if it's a file, check if it is a zip file
		//else if (entry->getType() == VFSEntry::EntryType::File)
		//{
		//	// convert to FileEntry
		//	std::unique_ptr<FileEntry> filePtr(static_cast<FileEntry*>(entry.release()));
		//	// check the file extension to see if it is a zip file
		//	if (filePtr->getPath().extension() == ".zip")
		//	{
		//		_scanner.addScanFolder(filePtr);
		//	}
		//}
	}

	return true;
}


} // namespace OpenXcom
