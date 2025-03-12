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
#include <filesystem>

namespace OpenXcom
{

class FileSystem;
class VFSEntry;

// Iterator implementation for the filesystem or folder
class FileSystemIteratorImpl
{
public:
	virtual ~FileSystemIteratorImpl() = default;

	virtual std::unique_ptr<VFSEntry> dereference() const = 0;
	virtual void increment() = 0;
	virtual bool equals(const FileSystemIteratorImpl& other) const = 0;
	virtual std::unique_ptr<FileSystemIteratorImpl> clone() const = 0;

	virtual bool isEnd() const = 0;
};

// Iterator for the filesystem or folder
class FileSystemIterator
{
private:
	bool isEnd() const
	{
		return (_impl == nullptr) || _impl->isEnd();
	}

	std::unique_ptr<FileSystemIteratorImpl> _impl;

public:
	using iterator_category = std::input_iterator_tag;
	using value_type = std::unique_ptr<VFSEntry>;
	using difference_type = std::ptrdiff_t;
	using pointer = const value_type*;
	using reference = const value_type&;

	// Constructors for begin and end iterators
	FileSystemIterator(std::unique_ptr<FileSystemIteratorImpl> impl) : _impl(std::move(impl)) {}

	// Iterator operations
	std::unique_ptr<VFSEntry> operator*() const // Dereference
	{
		if (!_impl || _impl->isEnd())
		{
			throw std::out_of_range("Dereferencing end iterator");
		}
		return _impl->dereference();
	}

	FileSystemIterator& operator++() // Pre-increment
	{
		if (!_impl || _impl->isEnd())
		{
			throw std::out_of_range("Incrementing end iterator");
		}

		_impl->increment();
		return *this;
	}

	FileSystemIterator operator++(int) // Post-increment
	{
		// Clone the current state of the iterator
		FileSystemIterator temp(_impl->clone());

		// Increment the current iterator
		++(*this);

		return temp;
	}


	bool operator==(const FileSystemIterator& other) const
	{
		// Both are end iterators
		if (isEnd() && other.isEnd())
		{
			return true;
		}

		// One is end and the other is not
		if (isEnd() || other.isEnd())
		{
			return false;
		}

		return _impl->equals(*other._impl);
	}

	bool operator!=(const FileSystemIterator& other) const { return !(other == *this); }
};



// Virtual File System Entry. Could be either a file or a directory
class VFSEntry
{
public:
	enum class EntryType
	{
		Directory,
		File,
		FileSystem
	};

	virtual ~VFSEntry() = default;

	virtual std::unique_ptr<VFSEntry> clone() const = 0;

	virtual EntryType getType() const = 0;

	// return the relative path from the root of the filesystem
	virtual std::filesystem::path getPath() const = 0;
};

// File Entry
class FileEntry : public VFSEntry
{
public:
	FileEntry() = default;
	virtual ~FileEntry() = default;

	virtual EntryType getType() const override { return EntryType::File; }

	virtual std::size_t getSize() const = 0;

	virtual std::unique_ptr<std::istream> openRead() = 0;
	virtual std::unique_ptr<std::ostream> openWrite() = 0;
	virtual std::unique_ptr<std::iostream> open() = 0;

protected:
};

class FolderEntry : public VFSEntry
{
public:
	virtual ~FolderEntry() = default;

	virtual EntryType getType() const override { return EntryType::Directory; }

	virtual std::unique_ptr<FileEntry> getFile(const std::filesystem::path& path) = 0;

	// create a filesystem for the folder
	virtual std::unique_ptr<FileSystem> createFileSystem() const = 0;

	// iterator support
	virtual FileSystemIterator begin() = 0;
	virtual FileSystemIterator end() = 0;
};

// Interface for all filesystem types
class FileSystem
{
public:
	virtual ~FileSystem() = default;

	virtual std::filesystem::path getPath() = 0;

	virtual std::unique_ptr<FileEntry> getFile(const std::filesystem::path& path) = 0;
	virtual std::unique_ptr<FolderEntry> getFolder(const std::filesystem::path& path) = 0;

	// Iterator support
	virtual FileSystemIterator begin() = 0;
	virtual FileSystemIterator end() = 0;

	// Helper functions
	std::string getFileExtension(const std::filesystem::path& path) const;
};

} // namespace OpenXcom
