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

#include "PhysicalFileSystem.h"

#include <fstream>

namespace OpenXcom
{


class PhysicalFileIteratorImpl : public FileSystemIteratorImpl
{
public:
	PhysicalFileIteratorImpl(const std::filesystem::path& path) : _currentIt(path), _endIt()
	{
	}

	virtual ~PhysicalFileIteratorImpl() override = default;

	virtual std::unique_ptr<VFSEntry> dereference() const override;

	virtual void increment() override
	{
		++_currentIt;
	}

	virtual bool equals(const FileSystemIteratorImpl& other) const override
	{
		const PhysicalFileIteratorImpl* otherImpl = dynamic_cast<const PhysicalFileIteratorImpl*>(&other);
		return otherImpl && _currentIt == otherImpl->_currentIt;
	}

	virtual std::unique_ptr<FileSystemIteratorImpl> clone() const override
	{
		auto cloned = std::make_unique<PhysicalFileIteratorImpl>(_currentIt->path());
		cloned->_currentIt = _currentIt;
		cloned->_endIt = _endIt;

		if (_currentEntry)
		{
			// Assuming VFSEntry has a clone method
			cloned->_currentEntry = _currentEntry->clone();
		}

		return cloned;
	}


	virtual bool isEnd() const override
	{
		return _currentIt == _endIt;
	}

private:
	std::filesystem::directory_iterator _currentIt;
	std::filesystem::directory_iterator _endIt;

	mutable std::unique_ptr<VFSEntry> _currentEntry;
};

class PhysicalFileEntry : public FileEntry
{
private:
	std::filesystem::path _path;

public:
	PhysicalFileEntry(const std::filesystem::path& path) : _path(path) { }
	virtual ~PhysicalFileEntry() override = default;
		
	virtual std::size_t getSize() const override
	{
		return std::filesystem::file_size(_path);
	}

	virtual std::unique_ptr<std::istream> openRead() override
	{
		return std::unique_ptr<std::istream>(static_cast<std::istream*>(new std::ifstream(_path, std::ios::binary)));
	}

	virtual std::unique_ptr<std::ostream> openWrite() override
	{
		return std::unique_ptr<std::ostream>(static_cast<std::ostream*>(std::make_unique<std::ofstream>(_path).release()));
	}

	virtual std::unique_ptr<std::iostream> open() override
	{
		return std::unique_ptr<std::iostream>(static_cast<std::iostream*>(std::make_unique<std::fstream>(_path).release()));
	}

	virtual std::filesystem::path getPath() const override
	{
		return _path;
	}

	virtual std::unique_ptr<VFSEntry> clone() const override
	{
		return std::make_unique<PhysicalFileEntry>(_path);
	}
};

class PhysicalFolderEntry : public FolderEntry
{
private:
	std::filesystem::path _path;

public:
	PhysicalFolderEntry(const std::filesystem::path& path) : _path(path) {}
	virtual ~PhysicalFolderEntry() override = default;

	virtual std::unique_ptr<FileEntry> getFile(const std::filesystem::path& path) override
	{
		std::filesystem::path fullPath = _path / path;

		// check if file exists and is a file type
		if (std::filesystem::exists(fullPath) && std::filesystem::status(fullPath).type() == std::filesystem::file_type::regular)
		{
			return std::make_unique<PhysicalFileEntry>(fullPath);
		}

		return std::unique_ptr<FileEntry>();
	}

	// create a filesystem for the folder
	virtual std::unique_ptr<FileSystem> createFileSystem() const override
	{
		return std::make_unique<PhysicalFileSystem>(_path);
	}

	virtual std::filesystem::path getPath() const override
	{
		return _path;
	}

	virtual std::unique_ptr<VFSEntry> clone() const override
	{
		return std::make_unique<PhysicalFolderEntry>(_path);
	}

	// Iterator support
	virtual FileSystemIterator begin()
	{
		return FileSystemIterator(std::make_unique<PhysicalFileIteratorImpl>(_path));
	}

	virtual FileSystemIterator end()
	{
		return FileSystemIterator(nullptr);
	}
};

std::unique_ptr<VFSEntry> PhysicalFileIteratorImpl::dereference() const
{
	if (std::filesystem::is_directory(*_currentIt))
	{
		return std::make_unique<PhysicalFolderEntry>(*_currentIt);
	}
	else
	{
		return std::make_unique<PhysicalFileEntry>(*_currentIt);
	}
}

PhysicalFileSystem::PhysicalFileSystem(const std::filesystem::path& path) : _path(path)
{
}

PhysicalFileSystem::~PhysicalFileSystem()
{
}

std::unique_ptr<FileEntry> PhysicalFileSystem::getFile(const std::filesystem::path& path)
{
	std::filesystem::path fullPath = _path / path;

	// check if file exists and is a file type
	if (std::filesystem::exists(fullPath) && std::filesystem::status(fullPath).type() == std::filesystem::file_type::regular)
	{
		return std::make_unique<PhysicalFileEntry>(fullPath);
	}

	return std::unique_ptr<FileEntry>();
}

std::unique_ptr<FolderEntry> PhysicalFileSystem::getFolder(const std::filesystem::path& path)
{
	std::filesystem::path fullPath = _path / path;

	// check if file exists and is a file type
	if (std::filesystem::exists(fullPath) && std::filesystem::status(fullPath).type() == std::filesystem::file_type::directory)
	{
		return std::make_unique<PhysicalFolderEntry>(fullPath);
	}

	return std::unique_ptr<FolderEntry>();
}

FileSystemIterator PhysicalFileSystem::begin()
{
	return FileSystemIterator(std::make_unique<PhysicalFileIteratorImpl>(_path));
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
