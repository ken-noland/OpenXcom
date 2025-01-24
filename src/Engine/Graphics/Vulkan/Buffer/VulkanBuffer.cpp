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
#include "../VulkanContext.h"

namespace OpenXcom
{

vk::BufferUsageFlags VulkanBuffer::getUsageFlags(BufferUsage usage)
{
	switch(usage)
	{
	case BufferUsage::Vertex:
		return vk::BufferUsageFlagBits::eVertexBuffer;
	case BufferUsage::Index:
		return vk::BufferUsageFlagBits::eIndexBuffer;
	case BufferUsage::Uniform:
		return vk::BufferUsageFlagBits::eUniformBuffer;
	case BufferUsage::Storage:
		return vk::BufferUsageFlagBits::eStorageBuffer;
	}

	return vk::BufferUsageFlags();
}


VulkanHostBuffer::VulkanHostBuffer(VulkanContext& context, vk::DeviceSize size, BufferUsage usage)
	: VulkanBuffer(context, size)
{
	vk::BufferUsageFlags usageFlags = getUsageFlags(usage) | vk::BufferUsageFlagBits::eTransferSrc;

	// create a buffer that is only accessible by the CPU
	vk::BufferCreateInfo bufferInfo{};
	bufferInfo.size = _size;
	bufferInfo.usage = usageFlags;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
	allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

	VkBuffer buffer = nullptr;
	VkBufferCreateInfo createInfo = bufferInfo;
	vmaCreateBuffer(context.getAllocator(), &createInfo, &allocInfo, &buffer, &_allocation, nullptr);

	_buffer = buffer;
}

VulkanHostBuffer::~VulkanHostBuffer()
{
	vmaDestroyBuffer(_context.getAllocator(), _buffer, _allocation);
}

void* VulkanHostBuffer::map()
{
	void* mappedData = nullptr;
	vmaMapMemory(_context.getAllocator(), _allocation, &mappedData);
	return mappedData;
}

void VulkanHostBuffer::unmap()
{
	vmaUnmapMemory(_context.getAllocator(), _allocation);
}

void VulkanHostBuffer::copyTo(const void* data, size_t offset, size_t size)
{
	void* mappedData = map();
	memcpy(static_cast<char*>(mappedData) + offset, data, size);
	unmap();
}

VulkanDeviceBuffer::VulkanDeviceBuffer(VulkanContext& context, VulkanHostBuffer& hostBuffer, BufferUsage usage)
	: VulkanBuffer(context, hostBuffer.getSize())
{
	create(usage);

	// copy the host buffer to the device buffer
	update(hostBuffer);

}

VulkanDeviceBuffer::VulkanDeviceBuffer(VulkanContext& context, vk::DeviceSize size, BufferUsage usage)
	: VulkanBuffer(context, size)
{
	create(usage);
}

VulkanDeviceBuffer::~VulkanDeviceBuffer()
{
	vmaDestroyBuffer(_context.getAllocator(), _buffer, _allocation);
}

/// Create a buffer that is accessible by the GPU
void VulkanDeviceBuffer::create(BufferUsage usage)
{
	vk::BufferUsageFlags usageFlags = getUsageFlags(usage) | vk::BufferUsageFlagBits::eTransferDst;

	vk::BufferCreateInfo bufferInfo{};
	bufferInfo.size = _size;
	bufferInfo.usage = usageFlags;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

	VkBuffer buffer = nullptr;
	VkBufferCreateInfo createInfo = bufferInfo;
	vmaCreateBuffer(_context.getAllocator(), &createInfo, &allocInfo, &buffer, &_allocation, nullptr);

	_buffer = buffer;
}

void VulkanDeviceBuffer::update(VulkanHostBuffer& hostBuffer)
{
	//KN NOTE: It's possible that we could use a multithreaded version of this which allows us to push up the contents without
	// having to wait for the previous command. This would reduce load times, but at the cost of adding complexity.
	vk::Result result = vk::Result::eSuccess;

	vk::CommandBuffer& commandBuffer = _context.getTransferQueue().getCommandBuffer();

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

	vk::Queue& transferQueue = _context.getTransferQueue().getQueue();
	result = transferQueue.submit(1, &submitInfo, VK_NULL_HANDLE);

	transferQueue.waitIdle();
}


} // namespace OpenXcom
