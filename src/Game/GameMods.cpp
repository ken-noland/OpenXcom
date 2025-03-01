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
	: _context(context), _scanner(context.getEngineContext())
{
}

GameMods::~GameMods()
{
}

bool GameMods::load()
{
	// register all the required and optional game directories in the mod scanner
	if (!setupScanner()) { return false; }

	// scan all the directories
	if (!_scanner.scan()) { return false; }
	std::vector<ScannedMod> scannedMods = _scanner.takeScannedMods();

	// Okay, we now have a list of mods to work with. Now we need to filter them.

	std::vector<ScannedMod> masters;
	std::vector<ScannedMod> activeMods;

	// Step 1: move the masters to their own list
	for(std::vector<ScannedMod>::iterator it = scannedMods.begin(); it != scannedMods.end(); )
	{
		if (it->info.type == ModType::Master)
		{
			masters.push_back(std::move(*it));
			it = scannedMods.erase(it);
		}
		else
		{
			++it;
		}
	}

	// Step 2: move active mods to their own list
	std::vector<std::string> configActiveMods = _context.getEngineContext().getOptions().get<&GameOptions::_mods>();
	for (std::vector<ScannedMod>::iterator it = scannedMods.begin(); it != scannedMods.end();)
	{
		// find mod in config list
		std::vector<std::string>::iterator configIt = std::find(configActiveMods.begin(), configActiveMods.end(), it->info.id);
		if (configIt != configActiveMods.end())
		{
			activeMods.push_back(std::move(*it));
			// remove it from the config list so we don't search for it again.
			configActiveMods.erase(configIt);
			it = scannedMods.erase(it);
		}
		else
		{
			++it;
		}
	}

	// Step 3: verify that all active mods have their dependencies met
	for (std::vector<ScannedMod>::iterator it = activeMods.begin(); it != activeMods.end();)
	{
		bool dependenciesMet = true;
		for (DependencyExpression& dep : it->info.dependencies)
		{
			// check if the mod is loaded
			std::vector<ScannedMod>::iterator depIt = std::find_if(activeMods.begin(), activeMods.end(), [&dep](const ScannedMod& mod) { return mod.info.id == dep.mod; });
			if (depIt == activeMods.end())
			{
				// check if the mod is a master
				depIt = std::find_if(masters.begin(), masters.end(), [&dep](const ScannedMod& mod) { return mod.info.id == dep.mod; });
				if (depIt == masters.end())
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
			scannedMods.push_back(std::move(*it));

			// remove the mod from the active list
			it = activeMods.erase(it);
		}
		else
		{
			++it;
		}
	}

	return true;
}

bool GameMods::setupScanner()
{
	EngineContext& engineContext = _context.getEngineContext();
	VirtualFileSystem& vfs = engineContext.getVirtualFileSystem();

	FileSystem& dataFs = vfs.getDataFileSystem();
	FileSystem& userFs = vfs.getUserFileSystem();

	// xcom1 and xcom2 are hardcoded to exist in the <data filesystem>/core/xcom1
	// and <data filesystem>/core/xcom2 directories
	std::unique_ptr<FolderEntry> xcom1Path = dataFs.getFolder("core/xcom1");
	std::unique_ptr<FolderEntry> xcom2Path = dataFs.getFolder("core/xcom2");

	_scanner.addModFolder(xcom1Path);
	_scanner.addModFolder(xcom2Path);

	// there's only one user directory, so just add it directly
	_scanner.addScanFolder(userFs.getFolder("mods"));

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
				_scanner.addScanFolder(folderPtr);
			}
		}
	}

	return true;
}


} // namespace OpenXcom
