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
#include "VulkanImage.h"

#include "../VulkanContext.h"
#include "../VulkanCommand.h"

#include "../../Primitive/PrimitiveFactory.h"

#include "../../BufferManager.h"
#include "../../../EngineContext.h"
#include "../../../Resource/ResourceSystem.h"

#include "../Buffer/VulkanBuffer.h"

namespace OpenXcom
{

void transitionImageLayout(vk::CommandBuffer cmdBuffer,	vk::Image image, vk::ImageLayout oldLayout,	vk::ImageLayout newLayout);

VulkanRenderTarget::VulkanRenderTarget(VulkanContext& context, glm::ivec2 extent, ImageFormat format, glm::vec4 color)
	: _context(context), _allocation(nullptr), _extent(extent), _color(color), _image(nullptr), _imageView(nullptr), _renderPass(nullptr), _framebuffer(nullptr)
{
	// Create the render target image
	vk::ImageCreateInfo imageInfo{};
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.extent = vk::Extent3D(_extent.x, _extent.y, 1);
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

	// need to create the primitives factory
	_primitiveFactory = std::make_unique<PrimitiveFactory>(_context.getEngineContext(), *this);

	// Create the device buffer which contains the image information on GPU
	BufferManager& bufferManager = _context.getEngineContext().getResourceSystem().getBufferManager();
	_deviceImageData = bufferManager.createDeviceBuffer<glm::ivec2>(&_extent, 1, BufferUsage::Uniform);
}

VulkanRenderTarget::~VulkanRenderTarget()
{
	_context.getDevice().destroyFramebuffer(_framebuffer);
	_context.getDevice().destroyRenderPass(_renderPass);

	_context.getDevice().destroyImageView(_imageView);
	vmaDestroyImage(_context.getAllocator(), _image, _allocation);
}

void VulkanRenderTarget::copyFrom(HostImage& hostImage)
{
	throw new std::runtime_error("Not implemented");
}

void VulkanRenderTarget::copyTo(HostImage& image)
{
	VulkanHostImage& hostImage = static_cast<VulkanHostImage&>(image);
	vk::CommandBuffer cmdBuffer = _context.getGraphicsQueue().getCommandBuffer(0);

	// Begin recording commands
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	cmdBuffer.begin(beginInfo);

	// Transition image for reading
	transitionImageLayout(cmdBuffer, _image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferSrcOptimal);

	// Define buffer copy region
	vk::BufferImageCopy region{};
	region.bufferOffset = 0;
	region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageExtent = vk::Extent3D(_extent.x, _extent.y, 1);

	// Copy image to buffer
	cmdBuffer.copyImageToBuffer(
		_image, // Render target image
		vk::ImageLayout::eTransferSrcOptimal,
		hostImage.getBuffer(),
		1,
		&region);

	// Transition image back to readable state
	transitionImageLayout(cmdBuffer, _image, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	// End recording commands
	cmdBuffer.end();

	// Submit the command buffer and wait for completion
	vk::SubmitInfo submitInfo{};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	vk::Queue graphicsQueue = _context.getGraphicsQueue().getQueue();
	graphicsQueue.submit(submitInfo, nullptr);
	graphicsQueue.waitIdle();
}

void VulkanRenderTarget::beginRenderPass(GraphicsCommand& command)
{
	vk::CommandBuffer& vkCommand = static_cast<VulkanCommand&>(command).getCommandBuffer();

	int width = getWidth();
	int height = getHeight();

	vk::Viewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(width);
	viewport.height = static_cast<float>(height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCommand.setViewport(0, 1, &viewport);

	vk::Rect2D scissor = {};
	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = vk::Extent2D(width, height);

	vkCommand.setScissor(0, 1, &scissor);

	vk::ClearValue clearColor = vk::ClearColorValue(std::array<float, 4>{_color.r, _color.g, _color.b, _color.a});

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
