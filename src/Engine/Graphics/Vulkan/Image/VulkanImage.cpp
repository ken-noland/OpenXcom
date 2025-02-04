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
#include "VulkanImage.h"
#include "../VulkanContext.h"

#include "../../BufferManager.h"
#include "../../../EngineContext.h"
#include "../../../Resource/ResourceSystem.h"

#include "../Buffer/VulkanBuffer.h"

namespace OpenXcom
{

void transitionImageLayout(
	vk::CommandBuffer cmdBuffer,
	vk::Image image,
	vk::ImageLayout oldLayout,
	vk::ImageLayout newLayout)
{
	vk::ImageMemoryBarrier barrier{};
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.layerCount = 1;

	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = static_cast<vk::AccessFlags>(0);
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if (oldLayout == vk::ImageLayout::eGeneral && newLayout == vk::ImageLayout::eTransferSrcOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eMemoryRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && newLayout == vk::ImageLayout::eTransferSrcOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
		sourceStage = vk::PipelineStageFlagBits::eFragmentShader;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eTransferSrcOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else
	{
		throw std::runtime_error("Unsupported image layout transition.");
	}

	cmdBuffer.pipelineBarrier(
		sourceStage,
		destinationStage,
		{},
		0, nullptr,
		0, nullptr,
		1, &barrier);
}


VulkanHostImage::VulkanHostImage(VulkanContext& context, glm::ivec2 extent, ImageFormat format)
	: HostImage(ImageType::Texture), _context(context), _format(format), _extent(extent)
{
	int bytePerPixel = 0;
	switch(format)
	{
	case ImageFormat::R8G8B8A8:
		bytePerPixel = 4;
		break;
	case ImageFormat::R8G8B8:
		bytePerPixel = 3;
		break;
	case ImageFormat::R8:
		bytePerPixel = 1;
		break;
	default:
		throw std::runtime_error("Unsupported image format.");
	}

	// Step 1: Define Buffer Create Info
	vk::BufferCreateInfo bufferInfo{};
	bufferInfo.size = _extent.x * _extent.y * bytePerPixel; // Assuming 4 bytes per pixel (RGBA)
	bufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	// Step 2: Define Memory Allocation Info (Host Visible)
	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
	allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;


	// Step 3: Allocate Buffer Memory with VMA
	VkBuffer buffer;
	vmaCreateBuffer(_context.getAllocator(), reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo), &allocCreateInfo, &buffer, &_allocation, nullptr);
	_buffer = buffer;
}

VulkanHostImage::~VulkanHostImage()
{
	if (_buffer)
	{
		vmaDestroyBuffer(_context.getAllocator(), _buffer, _allocation);
	}
}

ImageFormat VulkanHostImage::getFormat() const
{
	return ImageFormat();
}

glm::ivec2 VulkanHostImage::getExtent() const
{
	return _extent;
}

uint32_t VulkanHostImage::getWidth() const
{
	return _extent.x;
}

uint32_t VulkanHostImage::getHeight() const
{
	return _extent.y;
}

void* VulkanHostImage::map()
{
	void* data;
	vmaMapMemory(_context.getAllocator(), _allocation, &data);
	return data;
}

void VulkanHostImage::unmap()
{
	vmaUnmapMemory(_context.getAllocator(), _allocation);
}

VulkanDeviceImage::VulkanDeviceImage(VulkanContext& context, glm::ivec2 extent, ImageFormat format)
	: DeviceImage(ImageType::Texture), _context(context), _format(format), _extent(extent), _currentLayout(vk::ImageLayout::eUndefined)
{
	// Step 1: Define Image Creation Info
	vk::ImageCreateInfo imageInfo{};
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.extent = vk::Extent3D(_extent.x, _extent.y, 1);
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = vk::Format::eR8G8B8A8Unorm; // 8-bit RGBA format
	imageInfo.samples = vk::SampleCountFlagBits::e1;
	imageInfo.tiling = vk::ImageTiling::eOptimal; // Optimized for GPU
	imageInfo.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;

	// Step 2: Define Memory Allocation (Device Local)
	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

	// Step 3: Allocate Image Memory with VMA
	VkImage rawImage;
	vmaCreateImage(_context.getAllocator(), reinterpret_cast<VkImageCreateInfo*>(&imageInfo), &allocCreateInfo, &rawImage, &_allocation, nullptr);
	_image = rawImage;

	// Step 4: Create Image View (For GPU Use)
	vk::ImageViewCreateInfo imageViewInfo{};
	imageViewInfo.image = _image;
	imageViewInfo.viewType = vk::ImageViewType::e2D;
	imageViewInfo.format = imageInfo.format;
	imageViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.layerCount = 1;

	_imageView = _context.getDevice().createImageView(imageViewInfo);

	// Create the device buffer which contains the image information on GPU
	BufferManager& bufferManager = _context.getEngineContext().getResourceSystem().getBufferManager();
	_deviceImageData = bufferManager.createDeviceBuffer<glm::ivec2>(&_extent, 1, BufferUsage::Uniform);
}

VulkanDeviceImage::VulkanDeviceImage(VulkanContext& context, VulkanHostImage& image)
	: VulkanDeviceImage(context, image.getExtent(), image.getFormat())
{
}

VulkanDeviceImage::~VulkanDeviceImage()
{
	if (_imageView)
	{
		_context.getDevice().destroyImageView(_imageView);
	}

	if (_image)
	{
		vmaDestroyImage(_context.getAllocator(), _image, _allocation);
	}
}

void VulkanDeviceImage::copyFrom(HostImage& image)
{
	VulkanHostImage& hostImage = static_cast<VulkanHostImage&>(image);

	vk::CommandBuffer cmdBuffer = _context.getGraphicsQueue().getCommandBuffer(0);

	// Begin recording commands
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	cmdBuffer.begin(beginInfo);

	// Transition device image to TRANSFER_DST layout
	transitionImageLayout(cmdBuffer, _image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferDstOptimal);

	// Define the region for copying from the buffer to the image
	vk::BufferImageCopy region{};
	region.bufferOffset = 0;
	region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageExtent = vk::Extent3D(_extent.x, _extent.y, 1);

	// Perform the buffer-to-image copy
	cmdBuffer.copyBufferToImage(
		hostImage.getBuffer(),
		_image,
		vk::ImageLayout::eTransferDstOptimal,
		1,
		&region);

	// Transition device image to SHADER_READ layout
	transitionImageLayout(cmdBuffer, _image, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	// End recording commands
	cmdBuffer.end();

	// Submit and wait
	vk::SubmitInfo submitInfo{};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	vk::Queue graphicsQueue = _context.getGraphicsQueue().getQueue();
	graphicsQueue.submit(submitInfo, nullptr);
	graphicsQueue.waitIdle();
}

void VulkanDeviceImage::copyTo(HostImage& hostImage)
{
}

ImageFormat VulkanDeviceImage::getFormat() const
{
	return ImageFormat();
}

glm::ivec2 VulkanDeviceImage::getExtent() const
{
	return _extent;
}

uint32_t VulkanDeviceImage::getWidth() const
{
	return _extent.x;
}

uint32_t VulkanDeviceImage::getHeight() const
{
	return _extent.y;
}

} // namespace OpenXcom
