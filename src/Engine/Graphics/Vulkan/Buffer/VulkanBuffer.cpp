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
#include "../../../Logger.h"

namespace OpenXcom
{

VulkanHostBuffer::VulkanHostBuffer(VulkanContext& context, VulkanDeviceBuffer& deviceBuffer)
	: VulkanHostBuffer(context, deviceBuffer.getElementSize(), deviceBuffer.getCount(), deviceBuffer.getUsage())
{
	// copy the device buffer to the host buffer
	copy(deviceBuffer);
}

VulkanHostBuffer::VulkanHostBuffer(VulkanContext& context, std::size_t elementSize, std::size_t count, BufferUsage usage)
	: HostBuffer(usage, elementSize), _context(context)
{
	assert(elementSize > 0);
	assert(count > 0);

	allocate(elementSize * count);
	_count = count;
}

VulkanHostBuffer::~VulkanHostBuffer()
{
	deallocate();
}

void VulkanHostBuffer::allocate(std::size_t size)
{
	_size = size;
	vk::BufferUsageFlags usageFlags = _context.getBufferUsageFlags(_usage) | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;

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
	vmaCreateBuffer(_context.getAllocator(), &createInfo, &allocInfo, &buffer, &_allocation, nullptr);

	_buffer = buffer;
	_allocatedSize = size;
}

void VulkanHostBuffer::deallocate()
{
	vmaDestroyBuffer(_context.getAllocator(), _buffer, _allocation);
}

void VulkanHostBuffer::resize(std::size_t count)
{
	std::size_t newSize = _elementSize * count;
	if (newSize <= _allocatedSize)
	{
		_count = count;
		_size = newSize;
	}
	else
	{
		deallocate();
		allocate(newSize);
	}

}

void VulkanHostBuffer::reserve(std::size_t count)
{
	std::size_t newSize = _elementSize * count;
	if (newSize > _allocatedSize)
	{
		deallocate();
		allocate(newSize);
	}
}

void VulkanHostBuffer::clear()
{
	_count = 0;
	_size = 0;
}

void VulkanHostBuffer::copy(DeviceBuffer& deviceBuffer)
{
	VulkanDeviceBuffer& vulkanBuffer = static_cast<VulkanDeviceBuffer&>(deviceBuffer);

	assert(vulkanBuffer.getElementSize() == _elementSize);

	resize(vulkanBuffer.getCount());

	vk::Result result = vk::Result::eSuccess;

	vk::CommandBuffer& commandBuffer = _context.getTransferQueue().getCommandBuffer();

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	result = commandBuffer.begin(&beginInfo);

	vk::BufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = _size;
	commandBuffer.copyBuffer(vulkanBuffer.getBuffer(), _buffer, 1, &copyRegion);

	commandBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vk::Queue& transferQueue = _context.getTransferQueue().getQueue();
	result = transferQueue.submit(1, &submitInfo, VK_NULL_HANDLE);

	transferQueue.waitIdle();
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

void VulkanHostBuffer::copy(const void* data, std::size_t size)
{
	// yeah, I'm not too happy with this function. I'd prefer to have a container style interface(push_back, etc) but
	// that's a bit more work than I have time for right now.

	void* mappedData = nullptr;

	assert(size % _elementSize == 0); // size must be a multiple of the element size

	if (size > _size)
	{
		resize(size / _elementSize);
	}

	_count = size / _elementSize;

	vmaMapMemory(_context.getAllocator(), _allocation, &mappedData);
	memcpy(mappedData, data, size);
	vmaUnmapMemory(_context.getAllocator(), _allocation);
}

VulkanDeviceBuffer::VulkanDeviceBuffer(VulkanContext& context, const VulkanHostBuffer& hostBuffer)
	: VulkanDeviceBuffer(context, hostBuffer.getElementSize(), hostBuffer.getCount(), hostBuffer.getUsage())
{
	// copy the host buffer to the device buffer
	copy(hostBuffer);
}

VulkanDeviceBuffer::VulkanDeviceBuffer(VulkanContext& context, const VulkanDeviceBuffer& deviceBuffer)
	: VulkanDeviceBuffer(context, deviceBuffer.getElementSize(), deviceBuffer.getCount(), deviceBuffer.getUsage())
{
	// copy the device buffer to the device buffer
	copy(deviceBuffer);
}

VulkanDeviceBuffer::VulkanDeviceBuffer(VulkanContext& context, std::size_t elementSize, std::size_t count, BufferUsage usage)
	: DeviceBuffer(usage, elementSize), _context(context)
{
	assert(elementSize > 0);
	assert(count > 0);

	allocate(elementSize * count);
	_count = count;
}

VulkanDeviceBuffer::~VulkanDeviceBuffer()
{
	deallocate();
}

void VulkanDeviceBuffer::allocate(std::size_t size)
{
	_size = size;

	vk::BufferUsageFlags usageFlags = _context.getBufferUsageFlags(_usage) | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;

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
	_allocatedSize = size;
}

void VulkanDeviceBuffer::deallocate()
{
	_context.getDevice().waitIdle();
	vmaDestroyBuffer(_context.getAllocator(), _buffer, _allocation);
}

void VulkanDeviceBuffer::resize(std::size_t count)
{
	std::size_t newSize = _elementSize * count;
	if (newSize <= _allocatedSize)
	{
		_count = count;
		_size = newSize;
	}
	else
	{
		deallocate();
		allocate(newSize);
	}
}

void VulkanDeviceBuffer::reserve(std::size_t count)
{
	std::size_t newSize = _elementSize * count;
	if (newSize > _allocatedSize)
	{
		deallocate();
		allocate(newSize);
	}
}

void VulkanDeviceBuffer::clear()
{
	_count = 0;
	_size = 0;
}

void VulkanDeviceBuffer::copy(const VulkanHostBuffer& hostBuffer)
{
	// KN NOTE: It's possible that we could use a multithreaded version of this which allows us to push up the contents without
	//  having to wait for the previous command. This would reduce load times, but at the cost of adding complexity.
	if(hostBuffer.getCount() == 0)
	{
		Log(LOG_DEBUG) << "Attempting to copy a zero sized buffer";
		return;
	}

	resize(hostBuffer.getCount());

	vk::Result result = vk::Result::eSuccess;

	vk::CommandBuffer& commandBuffer = _context.getTransferQueue().getCommandBuffer();

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	result = commandBuffer.begin(&beginInfo);
	if(result != vk::Result::eSuccess)
	{
		Log(LOG_ERROR) << "Failed to begin command buffer";
		return;
	}

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
	if (result != vk::Result::eSuccess)
	{
		Log(LOG_ERROR) << "Failed to submit command buffer";
		return;
	}
	transferQueue.waitIdle();
}

void VulkanDeviceBuffer::copy(const VulkanDeviceBuffer& deviceBuffer)
{
	if(deviceBuffer.getCount() == 0)
    {
        Log(LOG_DEBUG) << "Attempting to copy a zero sized buffer";
        return;
    }

    // Ensure the destination buffer is resized appropriately
    resize(deviceBuffer.getCount());

    vk::Result result = vk::Result::eSuccess;

    // Get a command buffer from the transfer queue
    vk::CommandBuffer& commandBuffer = _context.getTransferQueue().getCommandBuffer();

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    result = commandBuffer.begin(&beginInfo);
	if(result != vk::Result::eSuccess)
	{
		Log(LOG_ERROR) << "Failed to begin command buffer";
		return;
	}

    // Define the region to copy: entire buffer in this case
    vk::BufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = _size;
    commandBuffer.copyBuffer(deviceBuffer.getBuffer(), _buffer, 1, &copyRegion);

    commandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // Submit the command buffer to the transfer queue
    vk::Queue& transferQueue = _context.getTransferQueue().getQueue();
    result = transferQueue.submit(1, &submitInfo, VK_NULL_HANDLE);
	if (result != vk::Result::eSuccess)
	{
		Log(LOG_ERROR) << "Failed to submit command buffer";
		return;
	}

    transferQueue.waitIdle();
}

void VulkanDeviceBuffer::copy(const HostBuffer& buffer)
{
	const VulkanHostBuffer& hostBuffer = static_cast<const VulkanHostBuffer&>(buffer);
	copy(hostBuffer);
}

} // namespace OpenXcom
