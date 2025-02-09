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
#include <gtest/gtest.h>

#include "../../Engine/Filesystem/VirtualFileSystem.h"
#include "../../Engine/Options.h"

using namespace OpenXcom;

TEST(VirtualFileSystemTest, TestPhysicalFileEntry)
{
	std::filesystem::path path = TEST_DATA_DIR;
	std::filesystem::path dataPath = path / "Data";
	std::filesystem::path configPath = path / "Config";
	std::filesystem::path userPath = path / "User";

	std::vector<std::string> args = {"-data", dataPath.string(), "-config", configPath.string(), "-user", userPath.string(), "-headless"};
	Options options(args);

	VirtualFileSystem vfs(options);

	FileSystem& data = vfs.getDataFileSystem();
	FilePtr entry = data.getFile("test.txt");
	EXPECT_TRUE(entry);

	FileEntry::IStreamPtr stream = entry->openRead();
	EXPECT_TRUE(stream);
	EXPECT_TRUE(*stream);

	//check the contents of stream
	std::string contents;
	std::getline(*stream, contents);

	EXPECT_EQ(contents, "This file is here to check if file opening and reading works");

	// reset the file ptr(causes it clean up the file handles and close)
	entry.reset();
}


TEST(VirtualFileSystemTest, TestPhysicalFilesystemIterator)
{
	std::filesystem::path path = TEST_DATA_DIR;
	std::filesystem::path dataPath = path / "Data";
	std::filesystem::path configPath = path / "Config";
	std::filesystem::path userPath = path / "User";

	std::vector<std::string> args = {"-data", dataPath.string(), "-config", configPath.string(), "-user", userPath.string(), "-headless"};
	Options options(args);

	VirtualFileSystem vfs(options);

	FileSystem& data = vfs.getDataFileSystem();

	for(const VFSEntryPtr& entry : data)
	{
		EXPECT_TRUE(entry);
	}
}
