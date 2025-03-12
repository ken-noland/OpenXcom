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

namespace OpenXcom
{

class EmbeddedStreamBuf : public std::streambuf
{
public:
	EmbeddedStreamBuf(const char* data, size_t size)
	{
		char* start = const_cast<char*>(data);
		this->setg(start, start, start + size);
	}
};

class EmbeddedFileEntry : public FileEntry
{
public:
	EmbeddedFileEntry(const std::filesystem::path& path) { _path = path; }

	virtual std::unique_ptr<std::istream> openRead() override
	{
		throw std::runtime_error("IStreamPtr EmbeddedFileEntry::openRead() Not implemented");
		return nullptr;
	}

	virtual std::unique_ptr<std::ostream> openWrite() override
	{
		throw std::runtime_error("OStreamPtr EmbeddedFileEntry::openWrite() Not implemented");
		return nullptr;
	}

	virtual std::unique_ptr<std::iostream> open() override
	{
		throw std::runtime_error("IOStreamPtr EmbeddedFileEntry::open() Not implemented");
		return nullptr;
	}

	virtual std::filesystem::path getPath() const override
	{
		return _path;
	}

private:
	std::filesystem::path _path;
};

class EmbeddedFileIteratorImpl : public FileSystemIteratorImpl
{
public:
	EmbeddedFileIteratorImpl(const std::filesystem::path& path, const PhysicalFileSystem& filesystem) : _currentIt(path), _filesystem(filesystem)
	{
	}

	virtual std::unique_ptr<VFSEntry> dereference() const override
	{
		throw std::runtime_error("VFSEntryPtr EmbeddedFileIteratorImpl::dereference() Not implemented");
		return nullptr;
	}

	virtual void increment() override
	{
		++_currentIt;
	}

	virtual bool equals(const FileSystemIteratorImpl& other) const override
	{
		return true;
	}

private:
	std::filesystem::directory_iterator _currentIt;

	const PhysicalFileSystem& _filesystem;
	mutable std::unique_ptr<VFSEntry> _currentEntry;
};

EmbeddedFileSystem::EmbeddedFileSystem()
{
}

EmbeddedFileSystem::~EmbeddedFileSystem()
{
}

std::filesystem::path EmbeddedFileSystem::getPath()
{
	//TODO: return the path to the executable
	return "<embedded>";
}

std::unique_ptr<FileEntry> EmbeddedFileSystem::getFile(const std::filesystem::path& path)
{
	return std::unique_ptr<FileEntry>();
}

std::unique_ptr<FolderEntry> EmbeddedFileSystem::getFolder(const std::filesystem::path& path)
{
	return std::unique_ptr<FolderEntry>();
}

FileSystemIterator EmbeddedFileSystem::begin()
{
	return FileSystemIterator(nullptr);
}

FileSystemIterator EmbeddedFileSystem::end()
{
	return FileSystemIterator(nullptr);
}

} // namespace OpenXcom
