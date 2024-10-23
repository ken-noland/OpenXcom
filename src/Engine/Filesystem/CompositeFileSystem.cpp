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
#include <set>

namespace OpenXcom
{

class CompositeFileIteratorImpl : public FileSystemIteratorImpl
{
public:
	CompositeFileIteratorImpl(CompositeFilesystem::EntryMapPtr entries)
		: _entries(std::move(entries))
	{
		_currentIt = _entries->begin();
		_currentEntry = std::move(_currentIt->second);

		_endIt = _entries->end();
	}

	virtual ~CompositeFileIteratorImpl() override = default;


	virtual VFSEntryPtr dereference() const override
	{
		if (_currentIt != _endIt)
		{
			return std::move(_currentEntry);
		}
		throw std::out_of_range("Iterator out of range");
	}

	virtual void increment() override
	{
		if (_currentIt != _endIt)
		{
			++_currentIt;
			if (_currentIt != _endIt)
				_currentEntry = std::move(_currentIt->second);
			else
				_currentEntry.reset();
		}
	}

	virtual bool equals(const FileSystemIteratorImpl& other) const override
	{
		const auto* otherImpl = dynamic_cast<const CompositeFileIteratorImpl*>(&other);
		return otherImpl && _currentIt == otherImpl->_currentIt;
	}

	virtual bool isEnd() const override
	{
		return _currentIt == _endIt;
	}

private:
	CompositeFilesystem::EntryMapPtr _entries; // Store the map as a unique_ptr

	CompositeFilesystem::EntryMap::iterator _currentIt;
	mutable VFSEntryPtr _currentEntry;

	CompositeFilesystem::EntryMap::iterator _endIt;
};


CompositeFilesystem::CompositeFilesystem()
{
}

CompositeFilesystem::~CompositeFilesystem()
{
}

void CompositeFilesystem::addFileSystem(std::unique_ptr<FileSystem> fs)
{
	_filesystems.push_back(std::move(fs));
}

FilePtr CompositeFilesystem::getFile(const std::filesystem::path& path)
{
	// first come, first serve
	for (const auto& fs : _filesystems)
	{
		FilePtr file = fs->getFile(path);
		if (file)
		{
			return file;
		}
	}
	return FilePtr();
}

FolderPtr CompositeFilesystem::getFolder(const std::filesystem::path& path)
{
	return FolderPtr();
}

FileSystemIterator CompositeFilesystem::begin()
{
	EntryMapPtr entries = std::make_unique<EntryMap>();

	for (std::unique_ptr<FileSystem>& fs : _filesystems)
	{
		for(VFSEntryPtr vfsPtr : *fs)
		{
			std::filesystem::path path = vfsPtr->getPath();
			entries->insert(std::make_pair(path, std::move(vfsPtr)));
		}
	}

	return FileSystemIterator(std::make_shared<CompositeFileIteratorImpl>(std::move(entries)));
}

FileSystemIterator CompositeFilesystem::end()
{
	return FileSystemIterator(nullptr);
}

} // namespace OpenXcom
