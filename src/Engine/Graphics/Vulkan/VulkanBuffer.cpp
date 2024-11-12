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

#include "VulkanBuffer.h"

namespace OpenXcom
{

VulkanBufferFactory::VulkanBufferFactory(VmaAllocator allocator, vk::Device device, uint32_t transferQueueFamilyIndex)
	: _allocator(allocator), _device(device)
{
	// Create a command pool for transfer operations
	vk::CommandPoolCreateInfo poolInfo = {};
	poolInfo.queueFamilyIndex = transferQueueFamilyIndex;
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

	vk::CommandPool transferCommandPool;
	_commandPool = _device.createCommandPool(poolInfo);

	// Retrieve the transfer queue
	_transferQueue = _device.getQueue(transferQueueFamilyIndex, 0);
}

VulkanBufferFactory::~VulkanBufferFactory()
{
	_device.destroyCommandPool(_commandPool);
	_commandPool = nullptr;
}

std::unique_ptr<VulkanHostBuffer> VulkanBufferFactory::createHostBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage)
{
	return std::make_unique<VulkanHostBuffer>(_allocator, usage, size);
}

std::unique_ptr<VulkanDeviceBuffer> VulkanBufferFactory::createDeviceBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage)
{
	return std::make_unique<VulkanDeviceBuffer>(_allocator, _device, _commandPool, _transferQueue, usage, size);
}

std::unique_ptr<VulkanDeviceBuffer> VulkanBufferFactory::createDeviceBuffer(VulkanHostBuffer& hostBuffer, vk::BufferUsageFlags usage)
{
	return std::make_unique<VulkanDeviceBuffer>(_allocator, _device, _commandPool, _transferQueue, usage, hostBuffer);
}


VulkanHostBuffer::VulkanHostBuffer(VmaAllocator allocator, vk::BufferUsageFlags usage, vk::DeviceSize size)
	: VulkanBuffer(allocator, size)
{
	vk::BufferUsageFlags usageFlags = usage | vk::BufferUsageFlagBits::eTransferSrc;

	// create a buffer that is only accessible by the CPU
	vk::BufferCreateInfo bufferInfo{};
	bufferInfo.size = _size;
	bufferInfo.usage = usageFlags;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

	VkBuffer buffer = nullptr;
	VkBufferCreateInfo createInfo = bufferInfo;
	vmaCreateBuffer(_allocator, &createInfo, &allocInfo, &buffer, &_allocation, nullptr);

	_buffer = buffer;
}

VulkanHostBuffer::~VulkanHostBuffer()
{
	vmaDestroyBuffer(_allocator, _buffer, _allocation);
}

void* VulkanHostBuffer::map()
{
	void* mappedData = nullptr;
	vmaMapMemory(_allocator, _allocation, &mappedData);
	return mappedData;
}

void VulkanHostBuffer::unmap()
{
	vmaUnmapMemory(_allocator, _allocation);
}

void VulkanHostBuffer::copyTo(const void* data, size_t offset, size_t size)
{
	void* mappedData = map();
	memcpy(static_cast<char*>(mappedData) + offset, data, size);
	unmap();
}

VulkanDeviceBuffer::VulkanDeviceBuffer(VmaAllocator allocator, vk::Device device, vk::CommandPool commandPool, vk::Queue transferQueue, vk::BufferUsageFlags usage, VulkanHostBuffer& hostBuffer)
	: VulkanBuffer(allocator, hostBuffer.getSize()), _device(device), _commandPool(commandPool), _transferQueue(transferQueue)
{
	create(usage);

	// copy the host buffer to the device buffer
	update(hostBuffer);

}

VulkanDeviceBuffer::VulkanDeviceBuffer(VmaAllocator allocator, vk::Device device, vk::CommandPool commandPool, vk::Queue transferQueue, vk::BufferUsageFlags usage, vk::DeviceSize size)
	: VulkanBuffer(allocator, size), _device(device), _commandPool(commandPool), _transferQueue(transferQueue)
{
	create(usage);
}

VulkanDeviceBuffer::~VulkanDeviceBuffer()
{
	vmaDestroyBuffer(_allocator, _buffer, _allocation);
}

/// Create a buffer that is accessible by the GPU
void VulkanDeviceBuffer::create(vk::BufferUsageFlags usage)
{
	vk::BufferUsageFlags usageFlags = usage | vk::BufferUsageFlagBits::eTransferDst;

	vk::BufferCreateInfo bufferInfo{};
	bufferInfo.size = _size;
	bufferInfo.usage = usageFlags;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkBuffer buffer = nullptr;
	VkBufferCreateInfo createInfo = bufferInfo;
	vmaCreateBuffer(_allocator, &createInfo, &allocInfo, &buffer, &_allocation, nullptr);

	_buffer = buffer;
}

void VulkanDeviceBuffer::update(VulkanHostBuffer& hostBuffer)
{
	//KN NOTE: It's possible that we could use a multithreaded version of this which allows us to push up the contents without
	// having to wait for the previous command. This would reduce load times, but at the cost of adding complextiy.
	vk::Result result = vk::Result::eSuccess;

	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandPool = _commandPool;
	allocInfo.commandBufferCount = 1;

	std::vector<vk::CommandBuffer> commandBuffers = _device.allocateCommandBuffers(allocInfo);
	vk::CommandBuffer& commandBuffer = commandBuffers[0];

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	result = commandBuffer.begin(&beginInfo);

	vk::BufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = _size;
	commandBuffer.copyBuffer(hostBuffer.getBuffer(), _buffer, 1, &copyRegion);

	commandBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	result = _transferQueue.submit(1, &submitInfo, VK_NULL_HANDLE);

	_transferQueue.waitIdle();

	_device.freeCommandBuffers(_commandPool, commandBuffers);
}



} // namespace OpenXcom
