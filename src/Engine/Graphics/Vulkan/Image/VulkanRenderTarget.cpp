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

#include "../../Buffer/BufferManager.h"
#include "../../../EngineContext.h"
#include "../../../Resource/ResourceSystem.h"

#include "../Buffer/VulkanBuffer.h"

namespace OpenXcom
{

void transitionImageLayout(vk::CommandBuffer cmdBuffer,	vk::Image image, vk::ImageLayout oldLayout,	vk::ImageLayout newLayout);

VulkanRenderTarget::VulkanRenderTarget(VulkanContext& context, glm::ivec2 extent, ImageFormat format, glm::vec4 color)
	: _context(context), _image{nullptr, nullptr}, _imageView{nullptr, nullptr}, _allocation{nullptr, nullptr}, _extent(extent), _color(color), _renderPass(nullptr), _framebuffer(nullptr)
{
	create();
}

VulkanRenderTarget::~VulkanRenderTarget()
{
	destroy();
}

void VulkanRenderTarget::createRenderSampleImage()
{
	// Create the render target image
	vk::ImageCreateInfo imageInfo{};
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.extent = vk::Extent3D(_extent.x, _extent.y, 1);
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = vk::Format::eR8G8B8A8Unorm; // 8-bit color with alpha
	imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc;
	imageInfo.samples = getSampleCountFlagBits();

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

	VkImage image;
	VmaAllocation allocation;

	vmaCreateImage(_context.getAllocator(), reinterpret_cast<VkImageCreateInfo*>(&imageInfo), &allocCreateInfo, &image, &allocation, nullptr);

	_image[0] = image;
	_allocation[0] = allocation;

	vk::ImageViewCreateInfo imageViewInfo{};
	imageViewInfo.image = _image[0];
	imageViewInfo.viewType = vk::ImageViewType::e2D;
	imageViewInfo.format = vk::Format::eR8G8B8A8Unorm;
	imageViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.layerCount = 1;

	_imageView[0] = _context.getDevice().createImageView(imageViewInfo);
}

void VulkanRenderTarget::createSingleSampleImage()
{
	// Create the render target image
	vk::ImageCreateInfo imageInfo{};
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.extent = vk::Extent3D(_extent.x, _extent.y, 1);
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = vk::Format::eR8G8B8A8Unorm; // 8-bit color with alpha
	imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc;
	imageInfo.samples = vk::SampleCountFlagBits::e1;

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

	VkImage image;
	VmaAllocation allocation;

	vmaCreateImage(_context.getAllocator(), reinterpret_cast<VkImageCreateInfo*>(&imageInfo), &allocCreateInfo, &image, &allocation, nullptr);

	_image[1] = image;
	_allocation[1] = allocation;

	vk::ImageViewCreateInfo imageViewInfo{};
	imageViewInfo.image = _image[1];
	imageViewInfo.viewType = vk::ImageViewType::e2D;
	imageViewInfo.format = vk::Format::eR8G8B8A8Unorm;
	imageViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.layerCount = 1;

	_imageView[1] = _context.getDevice().createImageView(imageViewInfo);
}

void VulkanRenderTarget::createMultisampled()
{
	createRenderSampleImage();
	createSingleSampleImage();

	vk::AttachmentDescription colorAttachment{};
	colorAttachment.format = vk::Format::eR8G8B8A8Unorm;
	colorAttachment.samples = getSampleCountFlagBits();
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentDescription resolveAttachment{};
	resolveAttachment.format = vk::Format::eR8G8B8A8Unorm;
	resolveAttachment.samples = vk::SampleCountFlagBits::e1;
	resolveAttachment.loadOp = vk::AttachmentLoadOp::eDontCare;
	resolveAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	resolveAttachment.initialLayout = vk::ImageLayout::eUndefined;
	resolveAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	vk::AttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0; // Index of the attachment in the render pass (color attachment)
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference resolveAttachmentRef{};
	resolveAttachmentRef.attachment = 1;
	resolveAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription subpass{};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pResolveAttachments = &resolveAttachmentRef;

	std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment, resolveAttachment};

	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	_renderPass = _context.getDevice().createRenderPass(renderPassInfo);

	vk::FramebufferCreateInfo framebufferInfo{};
	framebufferInfo.renderPass = _renderPass;
	framebufferInfo.attachmentCount = static_cast<uint32_t>(_imageView.size());
	framebufferInfo.pAttachments = _imageView.data();
	framebufferInfo.width = _extent.x;
	framebufferInfo.height = _extent.y;
	framebufferInfo.layers = 1;

	_framebuffer = _context.getDevice().createFramebuffer(framebufferInfo);

	// need to create the primitives factory
	_primitiveFactory = std::make_unique<PrimitiveFactory>(_context.getEngineContext(), *this);

	// Create the device buffer which contains the image information on GPU
	BufferManager& bufferManager = _context.getEngineContext().getResourceSystem().getBufferManager();
	_deviceImageData = bufferManager.createDeviceBuffer<glm::ivec2>(&_extent, 1, BufferUsage::Uniform);
}

void VulkanRenderTarget::createStandard()
{
	createRenderSampleImage();
	_image[1] = _image[0];
	_imageView[1] = _imageView[0];

	
	vk::AttachmentDescription colorAttachment{};
	colorAttachment.format = vk::Format::eR8G8B8A8Unorm;
	colorAttachment.samples = getSampleCountFlagBits();
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

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
	framebufferInfo.pAttachments = &_imageView[0];
	framebufferInfo.width = _extent.x;
	framebufferInfo.height = _extent.y;
	framebufferInfo.layers = 1;

	_framebuffer = _context.getDevice().createFramebuffer(framebufferInfo);

	// need to create the primitives factory
	_primitiveFactory = std::make_unique<PrimitiveFactory>(_context.getEngineContext(), *this);

	// Create the device buffer which contains the image information on GPU
	BufferManager& bufferManager = _context.getEngineContext().getResourceSystem().getBufferManager();
	_deviceImageData = bufferManager.createDeviceBuffer<glm::ivec2>(&_extent, 1, BufferUsage::Uniform);
}

void VulkanRenderTarget::create()
{
	if (getMultisampleCount() > 1)
	{
		createMultisampled();
	}
	else
	{
		createStandard();
	}
}

void VulkanRenderTarget::destroy()
{
	_context.getDevice().waitIdle();

	_context.getDevice().destroyFramebuffer(_framebuffer);
	_context.getDevice().destroyRenderPass(_renderPass);

	if (_image[0] == _image[1])
	{
		// standard
		_context.getDevice().destroyImageView(_imageView[0]);

		vmaDestroyImage(_context.getAllocator(), _image[0], _allocation[0]);
	}
	else
	{
		// multisampled
		_context.getDevice().destroyImageView(_imageView[0]);
		_context.getDevice().destroyImageView(_imageView[1]);

		vmaDestroyImage(_context.getAllocator(), _image[0], _allocation[0]);
		vmaDestroyImage(_context.getAllocator(), _image[1], _allocation[1]);
	}
}

void VulkanRenderTarget::setExtent(const glm::ivec2& size)
{
	destroy();

	_extent = size;
	create();
}

void VulkanRenderTarget::copyFrom(HostImage& hostImage)
{
	throw new std::runtime_error("VulkanRenderTarget::copyFrom(HostImage& hostImage) Not implemented");
}

void VulkanRenderTarget::copyTo(HostImage& image)
{
	VulkanHostImage& hostImage = static_cast<VulkanHostImage&>(image);

	std::function<void()> commandBufferFunc = [this, &hostImage]()
	{
		vk::CommandBuffer cmdBuffer = _context.getGraphicsQueue().getCommandBuffer(0);

		// Begin recording commands
		vk::CommandBufferBeginInfo beginInfo{};
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		cmdBuffer.begin(beginInfo);

		// Transition image for reading
		transitionImageLayout(cmdBuffer, _image[1], vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferSrcOptimal);

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
			_image[1], // Render target image
			vk::ImageLayout::eTransferSrcOptimal,
			hostImage.getBuffer(),
			1,
			&region);

		// Transition image back to readable state
		transitionImageLayout(cmdBuffer, _image[1], vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

		// End recording commands
		cmdBuffer.end();

		// Submit the command buffer and wait for completion
		vk::SubmitInfo submitInfo{};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		vk::Queue graphicsQueue = _context.getGraphicsQueue().getQueue();
		graphicsQueue.submit(submitInfo, nullptr);
		graphicsQueue.waitIdle();
	};
	
	VulkanQueueThread& graphicsQueueThread = _context.getGraphicsQueueThread();
	std::future<void> future = graphicsQueueThread.enqueueTask(commandBufferFunc);
	future.wait();
}

void VulkanRenderTarget::beginRenderPass(GraphicsCommand& command)
{
	vk::CommandBuffer& vkCommand = static_cast<VulkanCommand&>(command).getCommandBuffer();

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
	vk::CommandBuffer& commandBuffer = static_cast<VulkanCommand&>(command).getCommandBuffer();
	commandBuffer.endRenderPass();

	// insert a pipeline barrier here to ensure the image has time to render
	if (getMultisampleCount() > 1)
	{
		vk::ImageSubresourceRange subresourceRange{
			vk::ImageAspectFlagBits::eColor, // aspect mask
			0,                               // baseMipLevel
			1,                               // levelCount
			0,                               // baseArrayLayer
			1                                // layerCount
		};

		vk::ImageMemoryBarrier imageBarrier{
			{},                                       // srcAccessMask: you might specify vk::AccessFlagBits::eColorAttachmentWrite here
			vk::AccessFlagBits::eColorAttachmentRead, // dstAccessMask: so that subsequent reads are safe
			vk::ImageLayout::eShaderReadOnlyOptimal,  // oldLayout: layout used in pass #1
			vk::ImageLayout::eShaderReadOnlyOptimal,  // newLayout: layout needed in pass #2 (or shader read, if that’s what you need)
			VK_QUEUE_FAMILY_IGNORED,                  // srcQueueFamilyIndex
			VK_QUEUE_FAMILY_IGNORED,                  // dstQueueFamilyIndex
			_image[1],                                // the image to transition
			subresourceRange                          // the subresource range that applies
		};

		commandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eColorAttachmentOutput, // src stage: finishing writes in pass #1
			vk::PipelineStageFlagBits::eColorAttachmentOutput, // dst stage: about to begin color writes in pass #2
			vk::DependencyFlags(),                             // dependency flags (e.g. vk::DependencyFlagBits::eByRegion if needed)
			0, nullptr,                                        // no global memory barriers
			0, nullptr,                                        // no buffer memory barriers
			1, &imageBarrier);                                 // the image memory barrier
	}
	else
	{
		vk::ImageSubresourceRange subresourceRange{
			vk::ImageAspectFlagBits::eColor, // aspect mask
			0,                               // baseMipLevel
			1,                               // levelCount
			0,                               // baseArrayLayer
			1                                // layerCount
		};

		vk::ImageMemoryBarrier imageBarrier{
			{},                                       // srcAccessMask: you might specify vk::AccessFlagBits::eColorAttachmentWrite here
			vk::AccessFlagBits::eColorAttachmentRead, // dstAccessMask: so that subsequent reads are safe
			vk::ImageLayout::eColorAttachmentOptimal, // oldLayout: layout used in pass #1
			vk::ImageLayout::eShaderReadOnlyOptimal,  // newLayout: layout needed in pass #2 (or shader read, if that’s what you need)
			VK_QUEUE_FAMILY_IGNORED,                  // srcQueueFamilyIndex
			VK_QUEUE_FAMILY_IGNORED,                  // dstQueueFamilyIndex
			_image[1],                                // the image to transition
			subresourceRange                          // the subresource range that applies
		};

		commandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eColorAttachmentOutput, // src stage: finishing writes in pass #1
			vk::PipelineStageFlagBits::eColorAttachmentOutput, // dst stage: about to begin color writes in pass #2
			vk::DependencyFlags(),                             // dependency flags (e.g. vk::DependencyFlagBits::eByRegion if needed)
			0, nullptr,                                        // no global memory barriers
			0, nullptr,                                        // no buffer memory barriers
			1, &imageBarrier);                                 // the image memory barrier
	}
}

} // namespace OpenXcom
