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
#include "Xcom1FileProcessor.h"
#include "../../Engine/EngineContext.h"
#include "../../Engine/Logger.h"
#include "../../Engine/Filesystem/VirtualFileSystem.h"
#include "../../Engine/Yaml.h"
#include "../../Engine/YamlFile.h"
#include "../../Engine/Mod/ModInfo.h"

namespace OpenXcom
{

Xcom1MasterFileProcessor::Xcom1MasterFileProcessor(EngineContext& engine)
	: _engine(engine)
{

}

Xcom1MasterFileProcessor::~Xcom1MasterFileProcessor()
{
}

bool Xcom1MasterFileProcessor::load(const std::filesystem::path& path)
{
	// Hardcode the path to the xcom1 master file
	std::filesystem::path masterPath = std::filesystem::path("core") / "xcom1";

	// From the VFS, get the folder entry for the xcom1 master file
	VirtualFileSystem& vfs = _engine.getVirtualFileSystem();
	std::unique_ptr<FolderEntry> folder = vfs.getDataFileSystem().getFolder(masterPath);
	if (!folder)
	{
		Log(LOG_ERROR) << "Failed to load xcom1 master file. Could not find directory: " << masterPath;
		return false;
	}

	// Get the metadata file from the master folder
	std::unique_ptr<FileEntry> metadataFile = folder->getFile("metadata.yml");
	if (!metadataFile)
	{
		// TODO: check for json file
		Log(LOG_ERROR) << "Failed to load xcom1 master file. Could not find metadata.yml in directory: " << masterPath;
		return false;
	}

	// Load the metadata file
	YamlFile masterYamlFile;
	ModInfo masterInfo = masterYamlFile.load<ModInfo>(metadataFile);

	return true;
}

bool Xcom1MasterFileProcessor::isValid(const std::filesystem::path& path)
{
	return false;
}

bool Xcom1MasterFileProcessor::loadVanillaResources()
{
	return false;
}

bool Xcom1MasterFileProcessor::loadBattlescapeResources()
{
	return false;
}

bool Xcom1MasterFileProcessor::loadExtraResources()
{
	return false;
}


} // namespace OpenXcom
