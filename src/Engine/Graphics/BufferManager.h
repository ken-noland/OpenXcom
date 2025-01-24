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
#include <memory>
#include "Buffer.h"

namespace OpenXcom
{

enum class BufferUsage;

class BufferManager
{
public:
	BufferManager() = default;
	virtual ~BufferManager() = default;

	// Create an empty host buffer
	virtual std::unique_ptr<HostBuffer> createHostBuffer(std::size_t size, BufferUsage usage) = 0;

	// Create a host buffer and copy data into it
	template <typename T>
	inline std::unique_ptr<HostBuffer> createHostBuffer(const T* data, size_t count, BufferUsage usage);

	// Create an empty device buffer
	virtual std::unique_ptr<DeviceBuffer> createDeviceBuffer(std::size_t size, BufferUsage usage) = 0;

	// Create a device buffer from a host buffer
	virtual std::unique_ptr<DeviceBuffer> createDeviceBuffer(HostBuffer& hostBuffer, BufferUsage usage) = 0;

	// Create a device buffer and copy data into it
	// WARNING: this function is not efficient for frequent updates. It creates a temporary host buffer to then upload the
	// data to the GPU. Use createHostBuffer and createDeviceBuffer instead if you need to update the buffer frequently.
	// Use this function to upload static data.
	template <typename T>
	inline std::unique_ptr<DeviceBuffer> createDeviceBuffer(const T* data, size_t count, BufferUsage usage);
};

template <typename T>
std::unique_ptr<HostBuffer> BufferManager::createHostBuffer(const T* data, size_t count, BufferUsage usage)
{
	size_t size = count * sizeof(T);
	std::unique_ptr<HostBuffer> hostBuffer = createHostBuffer(size, usage);
	hostBuffer->copyTo(data, 0, size);
	return hostBuffer;
}

template <typename T>
std::unique_ptr<DeviceBuffer> BufferManager::createDeviceBuffer(const T* data, size_t count, BufferUsage usage)
{
	size_t size = count * sizeof(T);
	std::unique_ptr<HostBuffer> hostBuffer = createHostBuffer<T>(data, count, usage);
	std::unique_ptr<DeviceBuffer> deviceBuffer = createDeviceBuffer(*hostBuffer, usage);
	return deviceBuffer;
}

} // namespace OpenXcom
