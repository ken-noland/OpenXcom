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

namespace OpenXcom
{

VulkanImageFactory::VulkanImageFactory(VulkanContext& context)
	: _context(context)
{
}

VulkanImageFactory::~VulkanImageFactory()
{
}

std::unique_ptr<VulkanRenderTargetImage> VulkanImageFactory::createRenderTarget(uint32_t width, uint32_t height, ImageFormat format)
{
	return std::make_unique<VulkanRenderTargetImage>(_context, width, height, format);
}

VulkanImage::VulkanImage(VulkanContext& context, uint32_t width, uint32_t height, vk::ImageUsageFlags flags)
	: _context(context)
{
	throw new std::runtime_error("Not implemented");

	//_image = _context.getDevice().createImage(imageInfo);

	// vk::MemoryRequirements memRequirements = _device.getImageMemoryRequirements(_gameImage);

	// uint32_t memoryType = std::numeric_limits<uint32_t>::max();
	// vk::PhysicalDeviceMemoryProperties memProperties = _physicalDevice.getMemoryProperties();
	// for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	//{
	//	if ((memRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) == vk::MemoryPropertyFlagBits::eDeviceLocal)
	//	{
	//		memoryType = i;
	//		break;
	//	}
	// }

	// if (memoryType == std::numeric_limits<uint32_t>::max())
	//{
	//	throw std::runtime_error("Failed to find suitable memory type for game image!");
	// }

	// vk::MemoryAllocateInfo allocInfo{};
	// allocInfo.allocationSize = memRequirements.size;
	// allocInfo.memoryTypeIndex = memoryType;

	//_gameImageMemory = _device.allocateMemory(allocInfo);	// should I be using VMA here?

	//_device.bindImageMemory(_gameImage, _gameImageMemory, 0);


}

VulkanImage::~VulkanImage()
{
	vmaDestroyImage(_context.getAllocator(), _image, _allocation);
}


VulkanRenderTargetImage::VulkanRenderTargetImage(VulkanContext& context, uint32_t width, uint32_t height, ImageFormat format)
	: _context(context)
{
	// Create the render target image
	vk::ImageCreateInfo imageInfo{};
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.extent = vk::Extent3D{width, height, 1};
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = vk::Format::eR8G8B8A8Unorm; // 8-bit color with alpha
	imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

	VkImage image;
	VmaAllocation allocation;

	vmaCreateImage(_context.getAllocator(), reinterpret_cast<VkImageCreateInfo*>(&imageInfo), &allocCreateInfo, &image, &allocation, nullptr);

	_image = image;
	_allocation = allocation;

	vk::ImageViewCreateInfo imageViewInfo{};
	imageViewInfo.image = _image;
	imageViewInfo.viewType = vk::ImageViewType::e2D;
	imageViewInfo.format = vk::Format::eR8G8B8A8Unorm;
	imageViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.layerCount = 1;

	_imageView = _context.getDevice().createImageView(imageViewInfo);

	vk::AttachmentDescription colorAttachment{};
	colorAttachment.format = vk::Format::eR8G8B8A8Unorm;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	vk::AttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0; // Index of the attachment in the render pass (color attachment)
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription subpass{};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	_renderPass = _context.getDevice().createRenderPass(renderPassInfo);

	vk::FramebufferCreateInfo framebufferInfo{};
	framebufferInfo.renderPass = _renderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = &_imageView;
	framebufferInfo.width = 320;
	framebufferInfo.height = 200;
	framebufferInfo.layers = 1;

	_framebuffer = _context.getDevice().createFramebuffer(framebufferInfo);
}

VulkanRenderTargetImage::~VulkanRenderTargetImage()
{
	_context.getDevice().destroyFramebuffer(_framebuffer);
	_context.getDevice().destroyRenderPass(_renderPass);

	_context.getDevice().destroyImageView(_imageView);
	vmaDestroyImage(_context.getAllocator(), _image, _allocation);
}

} // namespace OpenXcom
