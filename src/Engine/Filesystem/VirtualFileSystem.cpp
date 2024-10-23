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

#include "VirtualFileSystem.h"
#include "../Options.h"


namespace OpenXcom
{

ZipFileSystem::ZipFileSystem(const std::filesystem::path& path)
{

}

ZipFileSystem::~ZipFileSystem()
{
}

FilePtr ZipFileSystem::getFile(const std::filesystem::path& path)
{
	return FilePtr();
}

FolderPtr ZipFileSystem::getFolder(const std::filesystem::path& path)
{
	return FolderPtr();
}

FileSystemIterator ZipFileSystem::begin()
{
	return FileSystemIterator(nullptr);
}

FileSystemIterator ZipFileSystem::end()
{
	return FileSystemIterator(nullptr);
}


VirtualFileSystem::VirtualFileSystem(Options& options)
	: _data(), _mods()
{
	// use the paths provided by the game options to set up the initial file systems
	for (const std::filesystem::path& path : options.get<&GameOptions::_dataPath>())
	{
		// first check to see if the path exists
		if (!std::filesystem::exists(path))
		{
			Log(LOG_ERROR) << "Data path does not exist: " << path << std::endl;
			continue;
		}

		// check if it's a folder or a file
		if (std::filesystem::status(path).type() == std::filesystem::file_type::directory)
		{
			// if it's a folder, than it must be a physical filesystem
			_data.addFileSystem(std::make_unique<PhysicalFileSystem>(path));
		}
		else
		{
			// check to see if the extension is zip
			if (path.extension() == ".zip")
			{
				// if it's a zip file, then it must be a zip filesystem
				_data.addFileSystem(std::make_unique<ZipFileSystem>(path));
			}
			else
			{
				Log(LOG_ERROR) << "Unknown data file: " << path << std::endl;
			}
		}
	}

	// TODO: Commenting this out for now until I have time to implement embedded files
	//_data.addFileSystem(std::make_unique<EmbeddedFileSystem>());

	// we need to use the data filesystem in order to get the path to the built-in mods

	std::filesystem::path modPath = options.get<&GameOptions::_userPath>() / "mods";
	_mods.addFileSystem(std::make_unique<PhysicalFileSystem>(modPath));
}

VirtualFileSystem::~VirtualFileSystem()
{
}

FileSystem& VirtualFileSystem::getDataFileSystem()
{
	return _data;
}

FileSystem& VirtualFileSystem::getModFileSystem()
{
	return _mods;
}

} // namespace OpenXcom
