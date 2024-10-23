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
#include <map>
#include <memory>

namespace OpenXcom
{

class FileSystem;
class Options;


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

	virtual EntryType getType() const = 0;

	//return the relative path from the root of the filesystem
	virtual std::filesystem::path getPath() const = 0;
};

using VFSEntryPtr = std::unique_ptr<VFSEntry>;

// File Entry
class FileEntry : public VFSEntry
{
public:
	FileEntry() = default;
	virtual ~FileEntry() = default;

	virtual EntryType getType() const override { return EntryType::File; }

	using IStreamPtr = std::shared_ptr<std::istream>;
	using OStreamPtr = std::shared_ptr<std::ostream>;
	using IOStreamPtr = std::shared_ptr<std::iostream>;

	virtual IStreamPtr openRead() = 0;
	virtual OStreamPtr openWrite() = 0;
	virtual IOStreamPtr open() = 0;

protected:

};

using FilePtr = std::unique_ptr<FileEntry>;

class FolderEntry : public VFSEntry
{
public:
	virtual ~FolderEntry() = default;

	virtual EntryType getType() const override { return EntryType::Directory; }
};

using FolderPtr = std::unique_ptr<FolderEntry>;


class FileSystemIteratorImpl
{
public:
	virtual ~FileSystemIteratorImpl() = default;

	virtual VFSEntryPtr dereference() const = 0;
	virtual void increment() = 0;
	virtual bool equals(const FileSystemIteratorImpl& other) const = 0;

	virtual bool isEnd() const = 0;
};

class FileSystemIterator
{
public:
	using iterator_category = std::input_iterator_tag;
	using value_type = VFSEntryPtr;
	using difference_type = std::ptrdiff_t;
	using pointer = const VFSEntryPtr*;
	using reference = const VFSEntryPtr&;

	// Constructors for begin and end iterators
	FileSystemIterator(std::shared_ptr<FileSystemIteratorImpl> impl) : _impl(impl) {}

	// Iterator operations
	VFSEntryPtr operator*() const // Dereference
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
		FileSystemIterator temp = *this;
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

private:
	bool isEnd() const
	{
		return (_impl == nullptr) || _impl->isEnd();
	}

	std::shared_ptr<FileSystemIteratorImpl> _impl;
};


// Interface for all filesystem types
class FileSystem
{
public:
	virtual ~FileSystem() = default;

	virtual FilePtr getFile(const std::filesystem::path& path) = 0;
	virtual FolderPtr getFolder(const std::filesystem::path& path) = 0;

	// Iterator support
	virtual FileSystemIterator begin() = 0;
	virtual FileSystemIterator end() = 0;
};

// for files that are embedded in the executable
class EmbeddedFileSystem : public FileSystem
{
public:
	EmbeddedFileSystem();
	virtual ~EmbeddedFileSystem() override;

	virtual FilePtr getFile(const std::filesystem::path& path) override;
	virtual FolderPtr getFolder(const std::filesystem::path& path) override;

	virtual FileSystemIterator begin() override;
	virtual FileSystemIterator end() override;
};

// for files that exist on the physical filesystem
class PhysicalFileSystem : public FileSystem
{
public:
	PhysicalFileSystem(const std::filesystem::path& path);
	virtual ~PhysicalFileSystem() override;

	virtual FilePtr getFile(const std::filesystem::path& path) override;
	virtual FolderPtr getFolder(const std::filesystem::path& path) override;
		
	virtual FileSystemIterator begin() override;
	virtual FileSystemIterator end() override;

	std::filesystem::path fullPathToRelative(const std::filesystem::path& path) const;

private:
	std::filesystem::path _path;
};

// for files that are stored in a zip archive
class ZipFileSystem : public FileSystem
{
public:
	ZipFileSystem(const std::filesystem::path& path);
	virtual ~ZipFileSystem() override;

	virtual FilePtr getFile(const std::filesystem::path& path) override;
	virtual FolderPtr getFolder(const std::filesystem::path& path) override;

	virtual FileSystemIterator begin() override;
	virtual FileSystemIterator end() override;
};

// for files that could be stored in multiple filesystems
class CompositeFilesystem : public FileSystem
{
public:
	// Used for iterating over the filesystems
	using EntryMap = std::map<std::filesystem::path, VFSEntryPtr>;
	using EntryMapPtr = std::unique_ptr<EntryMap>;

	CompositeFilesystem();
	virtual ~CompositeFilesystem() override;

	void addFileSystem(std::unique_ptr<FileSystem> fs);

	virtual FilePtr getFile(const std::filesystem::path& path) override;
	virtual FolderPtr getFolder(const std::filesystem::path& path) override;

	virtual FileSystemIterator begin() override;
	virtual FileSystemIterator end() override;

private:
	std::vector<std::unique_ptr<FileSystem>> _filesystems;
};

// The virtual filesystem manages all other files systems
//
// The virtual filesystem is a way to abstract the filesystem so that the
// game can access files in a consistent way, regardless of where the files
// are stored. This allows the game to access files that are embedded in
// the executable, stored on the physical filesystem, or stored in a zip
// archive, without having to know where the files are stored.
//
// During initialization, it takes the paths provided for "data", "user",
// and "config" and creates the appropriate filesystems for each of them.
// For data, it creates a CompositeFilesystem that contains an
// EmbeddedFileSystem, (potentially) a few PhysicalFilesystems, and (potentially)
// a few ZipFilesystems. For user and config, it creates a PhysicalFileSystem.
// Data contains resources such as "common", "standard", as well as the games
// base data files for Xcom.
//
// There's also a special case here because "standard" is actually a list of
// mods. Special handling is done to allow "standard" to be included in the
// "mods" special composite filesystem. Because mods can be located in the
// user directory, either as folders or as zips, it also needs to be a special
// composite filesystem.
//
// For mods, they can only access files that are within their mod path, so the
// filesystem is locked to that path. This is to prevent mods from accessing
// files outside of their mod directory.
class VirtualFileSystem
{
public:
	VirtualFileSystem(Options& options);
	~VirtualFileSystem();

	FileSystem& getDataFileSystem();
	FileSystem& getModFileSystem();

protected:
	// there can only be one embedded filesystem
	EmbeddedFileSystem _embedded;

	// data and mods are composite filesystems, meaning they can contain
	// multiple filesystems(embedded, physical and zip filesystems)
	CompositeFilesystem _data;
	CompositeFilesystem _mods;
};

} // namespace OpenXcom
