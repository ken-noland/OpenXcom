#include "CompositeFileSystem.h"
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
	CompositeFileIteratorImpl(CompositeFileSystem::EntryMapPtr entries)
		: _entries(std::move(entries))
	{
		_currentIt = _entries->begin();
		_currentEntry = std::move(_currentIt->second);

		_endIt = _entries->end();
	}

	virtual ~CompositeFileIteratorImpl() override = default;


	virtual std::unique_ptr<VFSEntry> dereference() const override
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

	virtual std::unique_ptr<FileSystemIteratorImpl> clone() const override
	{
		// Clone the entries map
		auto clonedEntries = std::make_unique<CompositeFileSystem::EntryMap>();

		// Deep clone each entry in the map
		for (const auto& [key, entry] : *_entries)
		{
			(*clonedEntries)[key] = entry ? entry->clone() : nullptr;
		}

		// Create a new iterator with the cloned map
		auto clonedIterator = std::make_unique<CompositeFileIteratorImpl>(std::move(clonedEntries));

		// Restore the iterator's position
		clonedIterator->_currentIt = clonedIterator->_entries->find(_currentIt->first);
		clonedIterator->_endIt = clonedIterator->_entries->end();

		// Clone the current entry if it exists
		if (_currentEntry)
		{
			clonedIterator->_currentEntry = _currentEntry->clone();
		}

		return clonedIterator;
	}

	virtual bool isEnd() const override
	{
		return _currentIt == _endIt;
	}

private:
	CompositeFileSystem::EntryMapPtr _entries; // Store the map as a unique_ptr

	CompositeFileSystem::EntryMap::iterator _currentIt;
	mutable std::unique_ptr<VFSEntry> _currentEntry;

	CompositeFileSystem::EntryMap::iterator _endIt;
};


CompositeFileSystem::CompositeFileSystem()
{
}

CompositeFileSystem::~CompositeFileSystem()
{
}

std::filesystem::path CompositeFileSystem::getPath()
{
	return "<composite>";
}

void CompositeFileSystem::addFileSystem(std::unique_ptr<FileSystem>&& fs)
{
	_filesystems.push_back(std::move(fs));
}

std::unique_ptr<FileEntry> CompositeFileSystem::getFile(const std::filesystem::path& path)
{
	// first come, first serve
	for (const auto& fs : _filesystems)
	{
		std::unique_ptr<FileEntry> file = fs->getFile(path);
		if (file)
		{
			return file;
		}
	}
	return std::unique_ptr<FileEntry>();
}

std::unique_ptr<FolderEntry> CompositeFileSystem::getFolder(const std::filesystem::path& path)
{
	// first come, first serve
	for (const auto& fs : _filesystems)
	{
		std::unique_ptr<FolderEntry> folder = fs->getFolder(path);
		if (folder)
		{
			return folder;
		}
	}
	return std::unique_ptr<FolderEntry>();
}

FileSystemIterator CompositeFileSystem::begin()
{
	EntryMapPtr entries = std::make_unique<EntryMap>();

	for (std::unique_ptr<FileSystem>& fs : _filesystems)
	{
		for (std::unique_ptr<VFSEntry> vfsPtr : *fs)
		{
			std::filesystem::path path = vfsPtr->getPath();
			entries->insert(std::make_pair(path, std::move(vfsPtr)));
		}
	}

	if (entries->empty())
	{
		return end();
	}

	return FileSystemIterator(std::make_unique<CompositeFileIteratorImpl>(std::move(entries)));
}

FileSystemIterator CompositeFileSystem::end()
{
	return FileSystemIterator(nullptr);
}

} // namespace OpenXcom
