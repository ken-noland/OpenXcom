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
#include "FileSystem.h"

namespace OpenXcom
{

// for files that are stored in a zip archive
class ZipFileSystem : public FileSystem
{
public:
	ZipFileSystem(const std::filesystem::path& path);
	virtual ~ZipFileSystem() override;

	virtual std::filesystem::path getPath() override;

	virtual std::unique_ptr<FileEntry> getFile(const std::filesystem::path& path) override;
	virtual std::unique_ptr<FolderEntry> getFolder(const std::filesystem::path& path) override;

	virtual FileSystemIterator begin() override;
	virtual FileSystemIterator end() override;
};

} // namespace OpenXcom
