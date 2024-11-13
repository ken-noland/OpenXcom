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

#include "VulkanResource.h"
#include "VulkanInclude.h"

#include <memory>

namespace OpenXcom
{

class VulkanHostBuffer;
class VulkanDeviceBuffer;

class VulkanBufferFactory
{
private:
	VmaAllocator _allocator;
	vk::Device _device;

	vk::CommandPool _commandPool;
	vk::Queue _transferQueue;


public:
	VulkanBufferFactory(VmaAllocator allocator, vk::Device device, uint32_t transferQueueFamilyIndex);
	~VulkanBufferFactory();

	// create an empty host buffer
	std::unique_ptr<VulkanHostBuffer> createHostBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage);

	// create a host buffer and copy data into it
	template <typename T>
	inline std::unique_ptr<VulkanHostBuffer> createHostBuffer(const T* data, size_t quantity, vk::BufferUsageFlags usage);

	// create an empty device buffer
	std::unique_ptr<VulkanDeviceBuffer> createDeviceBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage);

	// create a device buffer from a host buffer
	std::unique_ptr<VulkanDeviceBuffer> createDeviceBuffer(VulkanHostBuffer& hostBuffer, vk::BufferUsageFlags usage);

	// create a device buffer and copy data into it
	template <typename T>
	inline std::unique_ptr<VulkanDeviceBuffer> createDeviceBuffer(const T* data, size_t quantity, vk::BufferUsageFlags usage);
};

class VulkanBuffer
{
protected:
	VmaAllocator _allocator;
	VmaAllocation _allocation;

	vk::Buffer _buffer;
	vk::DeviceSize _size;

public:
	VulkanBuffer(VmaAllocator allocator, vk::DeviceSize size) : _allocator(allocator), _size(size) {}
	virtual ~VulkanBuffer() = default;

	vk::DeviceSize getSize() const { return _size; }
	vk::Buffer getBuffer() const { return _buffer; }
};

class VulkanHostBuffer : public VulkanBuffer
{
private:

public:
	VulkanHostBuffer(VmaAllocator allocator, vk::BufferUsageFlags usage, vk::DeviceSize size);
	virtual ~VulkanHostBuffer();

	void* map();
	void unmap();

	void copyTo(const void* data, size_t offset, size_t size);
};

class VulkanDeviceBuffer : public VulkanBuffer
{
private:
	vk::Device _device;
	vk::CommandPool _commandPool;
	vk::Queue _transferQueue;

	void create(vk::BufferUsageFlags usage);
	void update(VulkanHostBuffer& hostBuffer);

public:
	VulkanDeviceBuffer(VmaAllocator allocator, vk::Device device, vk::CommandPool commandPool, vk::Queue transferQueue, vk::BufferUsageFlags usage, VulkanHostBuffer& hostBuffer);
	VulkanDeviceBuffer(VmaAllocator allocator, vk::Device device, vk::CommandPool commandPool, vk::Queue transferQueue, vk::BufferUsageFlags usage, vk::DeviceSize size); // create a blank device buffer
	virtual ~VulkanDeviceBuffer();
};


template <typename T>
std::unique_ptr<VulkanHostBuffer> VulkanBufferFactory::createHostBuffer(const T* data, size_t quantity, vk::BufferUsageFlags usage)
{
	size_t size = quantity * sizeof(T);
	std::unique_ptr<VulkanHostBuffer> hostBuffer = createHostBuffer(size, usage);
	hostBuffer->copyTo(data, 0, size);
	return hostBuffer;
}

template <typename T>
std::unique_ptr<VulkanDeviceBuffer> VulkanBufferFactory::createDeviceBuffer(const T* data, size_t quantity, vk::BufferUsageFlags usage)
{
	size_t size = quantity * sizeof(T);
	std::unique_ptr<VulkanHostBuffer> hostBuffer = createHostBuffer<T>(data, quantity, usage);
	std::unique_ptr<VulkanDeviceBuffer> deviceBuffer = createDeviceBuffer(*hostBuffer, usage);
	return deviceBuffer;
}


} // namespace OpenXcom
