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

#include <fstream>

namespace OpenXcom
{

class PhysicalFileEntry : public FileEntry
{
public:
	PhysicalFileEntry(const std::filesystem::path& path) : _path(path) { }
	virtual ~PhysicalFileEntry() override = default;

	virtual IStreamPtr openRead() override
	{
		return std::unique_ptr<std::istream>(static_cast<std::istream*>(new std::ifstream(_path)));
	}

	virtual OStreamPtr openWrite() override
	{
		return std::unique_ptr<std::ostream>(static_cast<std::ostream*>(std::make_unique<std::ofstream>(_path).release()));
	}

	virtual IOStreamPtr open() override
	{
		return std::unique_ptr<std::iostream>(static_cast<std::iostream*>(std::make_unique<std::fstream>(_path).release()));
	}

	virtual std::filesystem::path getPath() const override
	{
		return _path;
	}

private:
	std::filesystem::path _path;
};

class PhysicalFileIteratorImpl : public FileSystemIteratorImpl
{
public:
	PhysicalFileIteratorImpl(const std::filesystem::path& path, const PhysicalFileSystem& filesystem) : _currentIt(path), _filesystem(filesystem), _endIt()
	{
	}

	virtual ~PhysicalFileIteratorImpl() override = default;


	virtual VFSEntryPtr dereference() const override
	{
		// from a full path, we want to get a relative path to the root of the filesystem
		return std::make_unique<PhysicalFileEntry>(_filesystem.fullPathToRelative(*_currentIt));
	}

	virtual void increment() override
	{
		++_currentIt;
	}

	virtual bool equals(const FileSystemIteratorImpl& other) const override
	{
		const PhysicalFileIteratorImpl* otherImpl = dynamic_cast<const PhysicalFileIteratorImpl*>(&other);
		return otherImpl && _currentIt == otherImpl->_currentIt;
	}

	virtual bool isEnd() const override
	{
		return _currentIt == _endIt;
	}

private:
	std::filesystem::directory_iterator _currentIt;
	std::filesystem::directory_iterator _endIt;

	const PhysicalFileSystem& _filesystem;
	mutable VFSEntryPtr _currentEntry;
};

PhysicalFileSystem::PhysicalFileSystem(const std::filesystem::path& path) : _path(path)
{
}

PhysicalFileSystem::~PhysicalFileSystem()
{
}

FilePtr PhysicalFileSystem::getFile(const std::filesystem::path& path)
{
	std::filesystem::path fullPath = _path / path;

	// check if file exists and is a file type
	if (std::filesystem::exists(fullPath) && std::filesystem::status(fullPath).type() == std::filesystem::file_type::regular)
	{
		return std::make_unique<PhysicalFileEntry>(fullPath);
	}

	return FilePtr();
}

FolderPtr PhysicalFileSystem::getFolder(const std::filesystem::path& path)
{
	return FolderPtr();
}

FileSystemIterator PhysicalFileSystem::begin()
{
	return FileSystemIterator(std::make_shared<PhysicalFileIteratorImpl>(_path, *this));
}

FileSystemIterator PhysicalFileSystem::end()
{
	return FileSystemIterator(nullptr);
}

std::filesystem::path PhysicalFileSystem::fullPathToRelative(const std::filesystem::path& path) const
{
	return std::filesystem::proximate(path, _path);
}

} // namespace OpenXcom
