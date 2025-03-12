#include "GameMods.h"
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
#include "../Engine/Filesystem/CompositeFileSystem.h"

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
	Options& options = _context.getEngineContext().getOptions();

	if(options.get<&GameOptions::_scanFilesystem>())
	{
		ModScanner scanner(_context.getEngineContext());

		// register all the required and optional game directories in the mod scanner
		if (!setupScanner(scanner))
		{
			return false;
		}

		// scan all the directories
		if (!scanner.scan())
		{
			return false;
		}
		_scannedMods.inactiveMods = scanner.takeScannedMods();
	}
	else
	{
		//TODO: if we're not scanning the filesystem for mods, we should probably add in the xcom1 and xcom2 masters?
	}

	// Okay, we now have a list of mods to work with. Now we need to filter them.

	// Step 1: move the masters to their own list
	for (std::vector<ScannedMod>::iterator it = _scannedMods.inactiveMods.begin(); it != _scannedMods.inactiveMods.end();)
	{
		if (it->info.type == ModType::Master)
		{
			_scannedMods.masters.push_back(std::move(*it));
			it = _scannedMods.inactiveMods.erase(it);
		}
		else
		{
			++it;
		}
	}

	// Step 2: Activate the master
	std::string masterId = options.get<&GameOptions::_master>();
	if (!activateMaster(masterId)) { return false; }

	// Step 3: Attempt to activate the mods
	std::vector<std::string> configActiveMods = options.get<&GameOptions::_mods>();
	std::vector<std::string> requeue;
	bool madeProgress = true;
	size_t maxPasses = configActiveMods.size() * 5;
	size_t passCount = 0;

	while(!configActiveMods.empty() && madeProgress && passCount < maxPasses)
	{
		madeProgress = false;
		for (std::vector<std::string>::iterator it = configActiveMods.begin(); it != configActiveMods.end();)
		{
			ActivateModResult result = activateMod(*it);
			if (result.result == ActivateModResult::ResultType::Success)
			{
				it = configActiveMods.erase(it);
				madeProgress = true;
				continue;
			}

			if(result.result == ActivateModResult::ResultType::Failed_MissingDependencies)
			{
				// This mod's dependencies aren't satisfied yet,
				// so we place it at the end of the list to try again later.
				// We'll keep going to the next mod, so we do:
				requeue.push_back(std::move(*it));
				it = configActiveMods.erase(it);
			}
			else
			{
				if (it != configActiveMods.end())
					++it;
			}
		}

		//requeue any mods with missing dependencies to allow other mods to load first
		configActiveMods.insert(configActiveMods.end(),
								std::make_move_iterator(requeue.begin()),
								std::make_move_iterator(requeue.end()));
		requeue.clear();

		++passCount;
	}
	// if there are any mods left, there must be an error
	if(!configActiveMods.empty())
	{
		Log(LOG_WARNING) << "Some mods could not be activated: " << configActiveMods.size();
		for (const std::string& modId : configActiveMods)
		{
			Log(LOG_WARNING) << "  " << modId;
			ActivateModResult result = activateMod(modId);
			if (result.result != ActivateModResult::ResultType::Success)
			{
				Log(LOG_WARNING) << "    " << result.failReason;

				if (result.result == ActivateModResult::ResultType::Failed_MissingDependencies)
				{
					for (const std::string& dep : result.errorDeps)
					{
						Log(LOG_WARNING) << "      " << dep;
					}
				}
				else if (result.result == ActivateModResult::ResultType::Failed_Conflicts)
				{
					for (const std::string& dep : result.errorDeps)
					{
						Log(LOG_WARNING) << "      " << dep;
					}
				}
			}
		}
		return false;
	}

	// Step 4: Load the mods
	Mod masterMod(_context.getEngineContext(), *_scannedMods.activeMaster);
	_loadedMods.mods.push_back(std::move(masterMod));
	_loadedMods.activeMaster = &_loadedMods.mods.back();

	for(ScannedMod& scannedMod : _scannedMods.activeMods)
	{
		Mod mod(_context.getEngineContext(), scannedMod);
		_loadedMods.mods.push_back(std::move(mod));
	}

	return true;
}

ActivateModResult GameMods::canActivateMod(const std::string& id) const
{
	ActivateModResult result;

	// Step 1: Already active?
	if (std::find_if(_scannedMods.activeMods.begin(), _scannedMods.activeMods.end(),
					 [&id](const ScannedMod& mod) { return mod.info.id == id; }) != _scannedMods.activeMods.end())
	{
		result.result = ActivateModResult::ResultType::AlreadyActive;
		return result;
	}

	// Step 2: Existence check
	std::vector<ScannedMod>::const_iterator it = std::find_if(_scannedMods.inactiveMods.begin(), _scannedMods.inactiveMods.end(),
														[&id](const ScannedMod& mod) { return mod.info.id == id; });
	if (it == _scannedMods.inactiveMods.end())
	{
		std::ostringstream os;
		os << "Mod " << id << " could not be located";
		result.result = ActivateModResult::ResultType::Failed_CouldNotLocate;
		result.failReason = os.str();
		return result;
	}

	// Step 3: Dependencies check
	const std::vector<DependencyExpression>& dependencies = it->info.dependencies;
	for (const DependencyExpression& dep : dependencies)
	{
		// TODO: Version check!

		// check if the requested mod is loaded
		std::vector<ScannedMod>::const_iterator depIt = std::find_if(_scannedMods.activeMods.begin(), _scannedMods.activeMods.end(),
															   [&dep](const ScannedMod& mod) { return mod.info.id == dep.mod; });
		if (depIt == _scannedMods.activeMods.end())
		{
			// check if the requested mod is the master
			assert(_scannedMods.activeMaster != nullptr);
			if (dep.mod != _scannedMods.activeMaster->info.id)
			{
				// dependency not met
				result.result = ActivateModResult::ResultType::Failed_MissingDependencies;
				result.errorDeps.push_back(dep.mod);
			}
		}
	}
	if (result.result == ActivateModResult::ResultType::Failed_MissingDependencies)
	{
		std::ostringstream os;
		os << "Mod " << id << " has unmet dependencies:";
		result.failReason = os.str();
		return result;
	}

	// Step 4: Conflicts check(check active mods)
	for (std::vector<ScannedMod>::const_iterator activeModIt = _scannedMods.activeMods.begin(); activeModIt != _scannedMods.activeMods.end(); ++activeModIt)
	{
		const std::vector<DependencyExpression>& conflicts = activeModIt->info.conflicts;
		for (const DependencyExpression& conflict : conflicts)
		{
			// check if the requested mod is loaded
			if (conflict.mod == id)
			{
				// conflict found
				result.result = ActivateModResult::ResultType::Failed_Conflicts;
				result.errorDeps.push_back((*activeModIt).info.id);
			}
		}
	}
	if (result.result == ActivateModResult::ResultType::Failed_Conflicts)
	{
		std::ostringstream os;
		os << "Mod " << id << " conflicts with the following mods:";
		result.failReason = os.str();
		return result;
	}

	// Step 5: Conflicts check(check inactive mods)
	const std::vector<DependencyExpression>& conflicts = it->info.conflicts;
	for (const DependencyExpression& conflict : conflicts)
	{
		// check if the requested mod is loaded
		std::vector<ScannedMod>::const_iterator conflictIt = std::find_if(_scannedMods.activeMods.begin(), _scannedMods.activeMods.end(),
																		  [&conflict](const ScannedMod& mod) { return mod.info.id == conflict.mod; });
		if (conflictIt != _scannedMods.activeMods.end())
		{
			// conflict found
			result.result = ActivateModResult::ResultType::Failed_Conflicts;
			result.errorDeps.push_back((*conflictIt).info.id);
		}
	}
	if (result.result == ActivateModResult::ResultType::Failed_Conflicts)
	{
		std::ostringstream os;
		os << "Mod " << id << " conflicts with the following mods:";
		result.failReason = os.str();
		return result;
	}

	// Step 6: All good!
	result.result = ActivateModResult::ResultType::Success;
	return result;
}

ActivateModResult GameMods::activateMod(const std::string& id)
{
	ActivateModResult result = canActivateMod(id);

	if(result.result == ActivateModResult::ResultType::Success)
	{
		std::vector<ScannedMod>::iterator it = std::find_if(_scannedMods.inactiveMods.begin(), _scannedMods.inactiveMods.end(),
															[&id](const ScannedMod& mod) { return mod.info.id == id; });

		// Activate the mod
		_scannedMods.activeMods.push_back(std::move(*it));
		_scannedMods.inactiveMods.erase(it);
	}

	return result;
}

bool GameMods::activateMaster(const std::string& id)
{
	// Step 1: Already active?
	if(_scannedMods.activeMaster != nullptr)
	{
		Log(LOG_ERROR) << "Master mod already active";
		return false;
	}

	// Step 2: Find the master
	std::vector<ScannedMod>::iterator it = std::find_if(_scannedMods.masters.begin(), _scannedMods.masters.end(),
														[&id](const ScannedMod& mod) { return mod.info.id == id; });
	if (it == _scannedMods.masters.end())
	{
		Log(LOG_ERROR) << "Master mod '" + id + "' not found";
		return false;
	}
	ScannedMod* master = &(*it);

	// Step 3: If the mod contains resourceDirectories, add them to the filesystem
	for(const std::filesystem::path& path : master->info.resourceDirectories)
	{
		// path is relative to the data folders
		VirtualFileSystem& vfs = _context.getEngineContext().getVirtualFileSystem();
		FileSystem& dataFs = vfs.getDataFileSystem();

		// check if folder exists
		std::unique_ptr<FolderEntry> folder = dataFs.getFolder(path);
		if(!folder)
		{
			Log(LOG_ERROR) << "Unable to locate resource data folder '" << path << "' for mod '" + id + "'";
			return false;
		}

		if(!master->filesystem)
		{
			master->filesystem = std::make_unique<CompositeFileSystem>();
		}

		master->filesystem->addFileSystem(folder->createFileSystem());
	}

	// Step 4: Activate the master
	_scannedMods.activeMaster = master;

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

			// check the directory name to see if it is "mods"
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
