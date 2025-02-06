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
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <cstdlib>
#include <vector>
#include <stdexcept>

namespace OpenXcom
{

enum class BufferUsage
{
	Vertex,
	Index,
	Uniform,
	Storage,
};

class DeviceBuffer;

class HostBuffer
{
protected:
	BufferUsage _usage;

	std::size_t _elementSize;
	std::size_t _count;

	std::size_t _allocatedSize;
	std::size_t _size;

public:
	HostBuffer(BufferUsage usage, std::size_t elementSize) : _usage(usage), _elementSize(elementSize) {};
	virtual ~HostBuffer() = default;

	const BufferUsage getUsage() const { return _usage; }

	std::size_t getCount() const { return _count; }

	std::size_t getSize() const { return _size; }

	std::size_t getElementSize() const { return _elementSize; }

	std::size_t getAllocatedSize() const { return _allocatedSize; }

	virtual void resize(std::size_t count) = 0;

	virtual void* map() = 0;
	virtual void unmap() = 0;


	virtual void copy(DeviceBuffer& deviceBuffer) = 0;

	virtual void copy(const void* data, std::size_t size) = 0;

    // Template helper to set data from a vector.
	template <typename T>
	inline void set(const std::vector<T>& data);
	
    // Template helper to set data from an initializer list.
	template <typename T>
	inline void set(std::initializer_list<T> data);
};

template <typename Type>
void HostBuffer::set(std::initializer_list<Type> data)
{
	// Verify that the element size matches.
	if (_elementSize != sizeof(Type))
		throw std::runtime_error("Element size mismatch in HostBuffer::set()");

	// Resize the buffer to hold the new count of elements.
	resize(data.size());
	// Copy data into the buffer.
	copy(static_cast<const void*>(data.data()), data.size());
}

template <typename Type>
void HostBuffer::set(const std::vector<Type>& data)
{
	// Verify that the element size matches.
	if (_elementSize != sizeof(Type))
		throw std::runtime_error("Element size mismatch in HostBuffer::set()");

	// Resize the buffer to hold the new count of elements.
	resize(data.size());
	// Copy data into the buffer.
	copy(static_cast<const void*>(data.data()), data.size());
}


class DeviceBuffer
{
protected:
	BufferUsage _usage;

	std::size_t _elementSize;
	std::size_t _count;

	std::size_t _allocatedSize;
	std::size_t _size;

public:
	DeviceBuffer(BufferUsage usage, std::size_t elementSize) : _usage(usage), _elementSize(elementSize) {}
	virtual ~DeviceBuffer() = default;

	const BufferUsage getUsage() const { return _usage; }

	std::size_t getCount() const { return _count; }

	std::size_t getSize() const { return _size; }

	std::size_t getElementSize() const { return _elementSize; }

	std::size_t getAllocatedSize() const { return _allocatedSize; }

	virtual void resize(std::size_t count) = 0;

	virtual void copy(const HostBuffer& buffer) = 0;
};

} // namespace OpenXcom
