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
#include <cassert>

namespace OpenXcom
{

enum class BufferUsage;

class BufferManager
{
public:
	BufferManager() = default;
	virtual ~BufferManager() = default;

	//TODO: Move over to handles!
	
	// Create an empty host buffer
	virtual std::unique_ptr<HostBuffer> createHostBuffer(std::size_t elementSize, std::size_t count, BufferUsage usage) = 0;

	// Create a host buffer from a device buffer
	virtual std::unique_ptr<HostBuffer> createHostBuffer(DeviceBuffer& deviceBuffer) = 0;

	// Create a host buffer and copy data into it
	template <typename BufferType>
	inline std::unique_ptr<HostBuffer> createHostBuffer(const BufferType* data, size_t count, BufferUsage usage);

	// Create a host buffer and copy data into it (std::vector variant)
	template <typename BufferType>
	inline std::unique_ptr<HostBuffer> createHostBuffer(const std::vector<BufferType>& data, BufferUsage usage);

	// Create a host buffer and copy data into it (std::initializer_list variant)
	template <typename BufferType>
	inline std::unique_ptr<HostBuffer> createHostBuffer(const std::initializer_list<BufferType>& data, BufferUsage usage);


	// Create an empty device buffer
	virtual std::unique_ptr<DeviceBuffer> createDeviceBuffer(std::size_t elementSize, std::size_t count, BufferUsage usage) = 0;

	// Create a device buffer from a host buffer
	virtual std::unique_ptr<DeviceBuffer> createDeviceBuffer(const HostBuffer& hostBuffer) = 0;

	// Create a device buffer from another device buffer
	virtual std::unique_ptr<DeviceBuffer> createDeviceBuffer(const DeviceBuffer& hostBuffer) = 0;

	// Create a device buffer and copy data into it
	// WARNING: this function is not efficient for frequent updates. It creates a temporary host buffer to then upload the
	// data to the GPU. Use createHostBuffer and createDeviceBuffer instead if you need to update the buffer frequently.
	// Use this function to upload static data.
	template <typename BufferType>
	inline std::unique_ptr<DeviceBuffer> createDeviceBuffer(const BufferType* data, size_t count, BufferUsage usage);
};

template <typename BufferType>
std::unique_ptr<HostBuffer> BufferManager::createHostBuffer(const BufferType* data, size_t count, BufferUsage usage)
{
	std::unique_ptr<HostBuffer> hostBuffer = createHostBuffer(sizeof(BufferType), count, usage);
	hostBuffer->copy(data, sizeof(BufferType) * count);
	return hostBuffer;
}

template <typename BufferType>
std::unique_ptr<HostBuffer> BufferManager::createHostBuffer(const std::vector<BufferType>& data, BufferUsage usage)
{
	std::unique_ptr<HostBuffer> hostBuffer = createHostBuffer(sizeof(BufferType), data.size(), usage);
	hostBuffer->copy(data.data(), sizeof(BufferType) * data.size());
	return hostBuffer;
}

template <typename BufferType>
std::unique_ptr<HostBuffer> BufferManager::createHostBuffer(const std::initializer_list<BufferType>& data, BufferUsage usage)
{
	std::unique_ptr<HostBuffer> hostBuffer = createHostBuffer(sizeof(BufferType), data.size(), usage);
	hostBuffer->copy(data.begin(), sizeof(BufferType) * data.size());
	return hostBuffer;
}

template <typename BufferType>
std::unique_ptr<DeviceBuffer> BufferManager::createDeviceBuffer(const BufferType* data, size_t count, BufferUsage usage)
{
	std::unique_ptr<HostBuffer> hostBuffer = createHostBuffer<BufferType>(data, count, usage);
	std::unique_ptr<DeviceBuffer> deviceBuffer = createDeviceBuffer(*hostBuffer);
	return deviceBuffer;
}

} // namespace OpenXcom
