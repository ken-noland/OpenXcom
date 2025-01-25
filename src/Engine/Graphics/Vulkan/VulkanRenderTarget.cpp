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
#include "VulkanRenderTarget.h"
#include "VulkanContext.h"
#include "VulkanCommand.h"

namespace OpenXcom
{

VulkanRenderTarget::VulkanRenderTarget(VulkanContext& context, uint32_t width, uint32_t height, ImageFormat format)
	: _context(context), _allocation(nullptr), _width(width), _height(height), _image(nullptr), _imageView(nullptr), _renderPass(nullptr), _framebuffer(nullptr)
{
	// Create the render target image
	vk::ImageCreateInfo imageInfo{};
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.extent = vk::Extent3D{_width, _height, 1};
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

VulkanRenderTarget::~VulkanRenderTarget()
{
	_context.getDevice().destroyFramebuffer(_framebuffer);
	_context.getDevice().destroyRenderPass(_renderPass);

	_context.getDevice().destroyImageView(_imageView);
	vmaDestroyImage(_context.getAllocator(), _image, _allocation);
}

void VulkanRenderTarget::beginRenderPass(GraphicsCommand& command)
{
	vk::CommandBuffer& vkCommand = static_cast<VulkanCommand&>(command).getCommandBuffer();
	vk::ClearValue clearColor = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 1.0f, 1.0f});

	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo.renderPass = _renderPass;
	renderPassInfo.framebuffer = _framebuffer;
	renderPassInfo.renderArea.extent = vk::Extent2D{getWidth(), getHeight()};
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCommand.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

}

void VulkanRenderTarget::endRenderPass(GraphicsCommand& command)
{
	vk::CommandBuffer& vkCommand = static_cast<VulkanCommand&>(command).getCommandBuffer();
	vkCommand.endRenderPass();
}

} // namespace OpenXcom
