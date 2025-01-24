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

#include "VulkanSurface.h"
#include "VulkanContext.h"
#include "VulkanCommand.h"

#include "Shader/VulkanShader.h"

#include "Pipeline/VulkanPipeline.h"

#include "../../Platform/Window.h"
#include "../../Engine.h"
#include "../../Resource/ResourceSystem.h"
#include "../../Logger.h"

#include <glm/gtc/matrix_transform.hpp>

// BEGIN TEMP
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <limits>

#undef None
#include "../../../Entity/Common/RTTR.h"


namespace OpenXcom
{



//Vertex vertices[] = {
//	{{-0.5f, -0.5f}, {0.0f, 0.0f}}, // Bottom-left, red
//	{{0.5f, -0.5f}, {1.0f, 0.0f}},  // Bottom-right, green
//	{{0.5f, 0.5f}, {1.0f, 1.0f}},   // Top-right, blue
//	{{-0.5f, 0.5f}, {0.0f, 1.0f}}   // Top-left, white
//};

//Vertex vertices[] = {
//	{{0.0f, 0.0f}, {0.0f, 0.0f}},     // Bottom-left corner
//	{{320.0f, 0.0f}, {1.0f, 0.0f}},   // Bottom-right corner
//	{{320.0f, 200.0f}, {1.0f, 1.0f}}, // Top-right corner
//	{{0.0f, 200.0f}, {0.0f, 1.0f}}    // Top-left corner
//};

//Vertex vertices[] = {
//	{{-1.0f, -1.0f}, {0.0f, 0.0f}}, // Bottom-left
//	{{1.0f, -1.0f}, {1.0f, 0.0f}},  // Bottom-right
//	{{1.0f, 1.0f}, {1.0f, 1.0f}},   // Top-right
//	{{-1.0f, 1.0f}, {0.0f, 1.0f}}   // Top-left
//};

// Indices for two triangles forming a rectangle
//uint16_t indices[] = {0, 1, 2, 2, 3, 0};

} // namespace OpenXcom


// END TEMP

namespace OpenXcom
{

VulkanSurface::VulkanSurface(VulkanContext& context, const PlatformWindowHandle& window)
	: _context(context), _surface(nullptr), _swapChain(nullptr), _swapChainImageFormat(vk::Format::eUndefined), _swapChainExtent{},
	_frames(), _currentFrame(0), _renderPass(nullptr), _windowHandle(window)
{
	_surface = createSurface(_context.getInstance(), window);

	initializeSwapChain();
	initializeRenderPass();
	initializeFrames();

	_commandContext = std::make_unique<VulkanCommand>(_context);
}

VulkanSurface::~VulkanSurface()
{
	destroySwapChain();

	if (_commandPool)
	{
		_context.getDevice().destroyCommandPool(_commandPool);
		_commandPool = nullptr;
	}

	if (_surface)
	{
		destroySurface(_context.getInstance(), _surface);
		_surface = nullptr;
	}
}

uint32_t VulkanSurface::getWidth()
{
	return _swapChainExtent.width;
}

uint32_t VulkanSurface::getHeight()
{
	return _swapChainExtent.height;
}

GraphicsCommand& VulkanSurface::beginCommandPass()
{
	// Acquire an image from the swap chain
	vk::Result result = vk::Result::eErrorUnknown;

    // Retry loop for acquireNextImageKHR
	while (true)
	{
		result = _context.getDevice().acquireNextImageKHR(
			_swapChain,
			UINT64_MAX, // Timeout
			_frames[_currentFrame].imageAvailableSemaphore,
			nullptr,
			&_imageIndex);

		if (result == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR)
		{
			// Successfully acquired an image
			break;
		}
		else if (result == vk::Result::eErrorOutOfDateKHR)
		{
			// Handle window resize (recreate swap chain)
			handleResize();

			// After handling the resize, try again
			continue;
		}
		else
		{
			// Unrecoverable error
			throw std::runtime_error("Failed to acquire swap chain image!");
		}
	}

	result = _context.getDevice().waitForFences(1, &_frames[_currentFrame].inFlightFence, VK_TRUE, UINT64_MAX);
	result = _context.getDevice().resetFences(1, &_frames[_currentFrame].inFlightFence);

	vk::CommandBuffer& commandBuffer = _frames[_currentFrame].commandBuffer;

	vk::CommandBufferBeginInfo beginInfo{};
	commandBuffer.begin(beginInfo);

	_commandContext->setCommandBuffer(commandBuffer);

	return *_commandContext;
}

void VulkanSurface::endCommandPass(GraphicsCommand& command)
{
	(void)command; // not using this parameter
	assert(&command == _commandContext.get());

	vk::CommandBuffer& commandBuffer = _frames[_currentFrame].commandBuffer;
	commandBuffer.end();

	// Use the current frame's synchronization objects
	vk::Semaphore waitSemaphores[] = {_frames[_currentFrame].imageAvailableSemaphore};
	vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
	vk::Semaphore signalSemaphores[] = {_frames[_currentFrame].renderFinishedSemaphore};

	// Submit the command buffer for execution
	vk::SubmitInfo submitInfo{};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vk::Result result = _context.getGraphicsQueue().getQueue().submit(1, &submitInfo, _frames[_currentFrame].inFlightFence);

	// Present the image
	vk::PresentInfoKHR presentInfo{};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores; // Wait for rendering to finish
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &_swapChain;
	presentInfo.pImageIndices = &_imageIndex; // Present the acquired image index

	result = _context.getPresentQueue().getQueue().presentKHR(&presentInfo);
	if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
	{
		handleResize(); // Handle window resize (recreate swapchain)
		return;
	}
	else if (result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to present swapchain image!");
	}
		
	// Increment the frame index, wrapping around the number of frames in flight
	_currentFrame = (_currentFrame + 1) % _frames.size();
}

void VulkanSurface::beginRenderPass(GraphicsCommand& command)
{
	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo.renderPass = _renderPass;
	renderPassInfo.framebuffer = _frames[_currentFrame].framebuffer;
	renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
	renderPassInfo.renderArea.extent = _swapChainExtent;

	vk::ClearValue clearColor = vk::ClearColorValue(std::array<float, 4>{0.2f, 0.2f, 0.2f, 1.0f});
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	_commandContext->getCommandBuffer().beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
}

void VulkanSurface::endRenderPass(GraphicsCommand& command)
{
	_commandContext->getCommandBuffer().endRenderPass();
}

// Function to get the client area dimensions
vk::Extent2D getClientAreaSize(const PlatformWindowHandle& handle)
{
#if defined(_WIN32)
	RECT clientRect;
	GetClientRect(handle.hWnd, &clientRect); // Retrieves the client area of the window
	return vk::Extent2D{
		static_cast<uint32_t>(clientRect.right - clientRect.left),
		static_cast<uint32_t>(clientRect.bottom - clientRect.top)};
#elif defined(__linux__)
	Window root;
	int x, y;
	unsigned int width, height, borderWidth, depth;

	// Get the geometry of the client area
	XGetGeometry((Display*)handle.display, handle.window, &root, &x, &y, &width, &height, &borderWidth, &depth);

	return vk::Extent2D{
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)};
#else
	#error "Unsupported platform"
#endif
}

void VulkanSurface::initializeSwapChain()
{
	// Get the surface capabilities, formats, and present modes
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = _context.getPhysicalDevice().getSurfaceCapabilitiesKHR(_surface);
	std::vector<vk::SurfaceFormatKHR> surfaceFormats = _context.getPhysicalDevice().getSurfaceFormatsKHR(_surface);
	std::vector<vk::PresentModeKHR> presentModes = _context.getPhysicalDevice().getSurfacePresentModesKHR(_surface);

	// Choose the best format
	vk::SurfaceFormatKHR chosenFormat = surfaceFormats[0];
	for (const auto& availableFormat : surfaceFormats)
	{
		if (availableFormat.format == vk::Format::eR8G8B8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			chosenFormat = availableFormat;
			break;
		}
	}

	// Choose the best presentation mode
	vk::PresentModeKHR chosenPresentMode = vk::PresentModeKHR::eFifo; // Always supported
	for (const auto& availablePresentMode : presentModes)
	{
		if (availablePresentMode == vk::PresentModeKHR::eMailbox)
		{
			chosenPresentMode = availablePresentMode;
			break;
		}
	}

	// Choose the best swap extent
	vk::Extent2D swapChainExtent = surfaceCapabilities.currentExtent;
	if (surfaceCapabilities.currentExtent.width == UINT32_MAX)
	{
		swapChainExtent = getClientAreaSize(_windowHandle); // Replace with actual window size
		swapChainExtent.width = std::clamp(swapChainExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
		swapChainExtent.height = std::clamp(swapChainExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
	}

	// now create the swap chain
	vk::SwapchainCreateInfoKHR swapChainCreateInfo{};
	swapChainCreateInfo.surface = _surface;
	swapChainCreateInfo.minImageCount = surfaceCapabilities.minImageCount + 1; // Add an extra image for triple buffering
	swapChainCreateInfo.imageFormat = chosenFormat.format;
	swapChainCreateInfo.imageColorSpace = chosenFormat.colorSpace;
	swapChainCreateInfo.imageExtent = swapChainExtent;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive; // Adjust for multiple queues if necessary
	swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
	swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapChainCreateInfo.presentMode = chosenPresentMode;
	swapChainCreateInfo.clipped = VK_TRUE;
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE; // Replace if resizing

	_swapChain = _context.getDevice().createSwapchainKHR(swapChainCreateInfo);
	_swapChainImageFormat = chosenFormat.format;
	_swapChainExtent = swapChainExtent;

	//// Define game and window dimensions
	//float gameWidth = 320.0f;
	//float gameHeight = 200.0f;
	//float windowWidth = static_cast<float>(_swapChainExtent.width);
	//float windowHeight = static_cast<float>(_swapChainExtent.height);

	//// Calculate aspect ratios
	//float gameAspectRatio = gameWidth / gameHeight;
	//float windowAspectRatio = windowWidth / windowHeight;

	//float scaleX = 1.0f;
	//float scaleY = 1.0f;

	//if (windowAspectRatio > gameAspectRatio)
	//{
	//	// Window is wider than game surface
	//	scaleX = gameAspectRatio / windowAspectRatio;
	//	// Centered horizontally in NDC, so no offset needed
	//}
	//else
	//{
	//	// Window is taller than game surface
	//	scaleY = windowAspectRatio / gameAspectRatio;
	//	// Centered vertically in NDC, so no offset needed
	//}

	////_transform = glm::mat4(1.0f);
	//_transform = glm::scale(glm::mat4(1.0f), glm::vec3(scaleX, scaleY, 1.0f));
}

void VulkanSurface::initializeRenderPass()
{
	// Initialize the render pass
	vk::AttachmentDescription colorAttachment{};
	colorAttachment.format = _swapChainImageFormat; // Format matching the swap chain
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR; // Used for presentation

	vk::AttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
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
}

void VulkanSurface::initializeFrames()
{
	// Retrieve swap chain images
	std::vector<vk::Image> swapChainImages = _context.getDevice().getSwapchainImagesKHR(_swapChain);
	std::size_t swapChainImageCount = swapChainImages.size();

	_frames.resize(swapChainImageCount);
	
	// Create the command queue
	vk::CommandPoolCreateInfo poolInfo{};
	poolInfo.queueFamilyIndex = _context.getGraphicsQueue().getFamilyIndex();
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	_commandPool = _context.getDevice().createCommandPool(poolInfo);

	// Allocate command buffer for this frame
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.commandPool = _commandPool;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandBufferCount = (uint32_t)swapChainImageCount;

	std::vector<vk::CommandBuffer> commandBuffers = _context.getDevice().allocateCommandBuffers(allocInfo);

	for (size_t i = 0; i < swapChainImageCount; ++i)
	{
		FrameData frameData{};

		// Create semaphores
		vk::SemaphoreCreateInfo semaphoreInfo{};
		frameData.imageAvailableSemaphore = _context.getDevice().createSemaphore(semaphoreInfo);
		frameData.renderFinishedSemaphore = _context.getDevice().createSemaphore(semaphoreInfo);

		// Create fence
		vk::FenceCreateInfo fenceInfo{};
		fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
		frameData.inFlightFence = _context.getDevice().createFence(fenceInfo);

		// Create image view for swap chain image
		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.image = swapChainImages[i]; // Assumes you've retrieved swap chain images
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = _swapChainImageFormat;
		viewInfo.components = {vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
							   vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity};
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		frameData.imageView = _context.getDevice().createImageView(viewInfo);

		// Create framebuffer for each swap chain image view
		vk::FramebufferCreateInfo framebufferInfo{};
		framebufferInfo.renderPass = _renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &frameData.imageView;
		framebufferInfo.width = _swapChainExtent.width;
		framebufferInfo.height = _swapChainExtent.height;
		framebufferInfo.layers = 1;
		frameData.framebuffer = _context.getDevice().createFramebuffer(framebufferInfo);

		frameData.commandBuffer = std::move(commandBuffers[i]);

		_frames[i] = std::move(frameData);
	}
}

//void VulkanSurface::initializeGameSurface(VulkanBufferFactory& bufferFactory)
//{
//	//// Update descriptor set
//	//vk::DescriptorImageInfo descImageInfo{};
//
//	//descImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
//	//descImageInfo.imageView = _gameImageView;
//	//descImageInfo.sampler = _textureSampler;
//
//	//vk::WriteDescriptorSet descriptorWrite{};
//
//	//descriptorWrite.dstSet = _descriptorSet;
//	//descriptorWrite.dstBinding = 0;
//	//descriptorWrite.dstArrayElement = 0;
//	//descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
//	//descriptorWrite.descriptorCount = 1;
//	//descriptorWrite.pImageInfo = &descImageInfo;
//
//	//_device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
//
//	////create the vertex and index buffer to present the game surface to the window surface
//	////_vertexBuffer = std::make_unique<VulkanBuffer>(_allocator, _device, (sizeof(vertices) * sizeof(Vertex)), vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
//	////_indexBuffer = std::make_unique<VulkanBuffer>(_allocator, _device, (sizeof(indices) * sizeof(uint16_t)), vk::BufferUsageFlagBits::eIndexBuffer, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
//
//	//_vertexBuffer = bufferFactory.createDeviceBuffer<Vertex>(vertices, 4, vk::BufferUsageFlagBits::eVertexBuffer);
//	//_indexBuffer = bufferFactory.createDeviceBuffer<uint16_t>(indices, 6, vk::BufferUsageFlagBits::eIndexBuffer);
//}

void VulkanSurface::destroySwapChain()
{
	if (_context.getDevice())
	{
		_context.getDevice().waitIdle();

		//_pipeline.reset();

		for (const FrameData& frame : _frames)
		{
			frame.commandBuffer.reset();
			_context.getDevice().destroyImageView(frame.imageView);
			_context.getDevice().destroyFramebuffer(frame.framebuffer);
			_context.getDevice().destroySemaphore(frame.imageAvailableSemaphore);
			_context.getDevice().destroySemaphore(frame.renderFinishedSemaphore);
			_context.getDevice().destroyFence(frame.inFlightFence);
		}
		_frames.clear();

		//reset the command pool
		_context.getDevice().resetCommandPool(_commandPool, vk::CommandPoolResetFlags());

		if (_swapChain)
		{
			_context.getDevice().destroySwapchainKHR(_swapChain);
			_swapChain = nullptr;
		}

		if (_renderPass)
		{
			_context.getDevice().destroyRenderPass(_renderPass);
			_renderPass = nullptr;
		}
	}
}

void VulkanSurface::handleResize()
{
	throw new std::runtime_error("Not implemented");
}


//void VulkanSurface::draw()
//{
//	uint32_t imageIndex = 0;
//
//	// Acquire an image from the swap chain
//	vk::Result result = _context.getDevice().acquireNextImageKHR(_swapChain, UINT64_MAX, _frames[_currentFrame].imageAvailableSemaphore, nullptr, &imageIndex);
//
//	if (result == vk::Result::eErrorOutOfDateKHR)
//	{
//		// Handle window resize (recreate swap chain)
//		handleResize();
//		return;
//	}
//	else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
//	{
//		throw std::runtime_error("Failed to acquire swap chain image!");
//	}
//
//	result = _context.getDevice().waitForFences(1, &_frames[_currentFrame].inFlightFence, VK_TRUE, UINT64_MAX);
//	result = _context.getDevice().resetFences(1, &_frames[_currentFrame].inFlightFence);
//
//	recordCommandBuffer(_frames[_currentFrame], imageIndex);
//
//	vk::Semaphore waitSemaphores[] = {_frames[_currentFrame].imageAvailableSemaphore};
//	vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
//	vk::Semaphore signalSemaphores[] = {_frames[_currentFrame].renderFinishedSemaphore};
//
//	vk::SubmitInfo submitInfo{};
//	submitInfo.waitSemaphoreCount = 1;
//	submitInfo.pWaitSemaphores = waitSemaphores;
//	submitInfo.pWaitDstStageMask = waitStages;
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = &_frames[_currentFrame].commandBuffer;
//	submitInfo.signalSemaphoreCount = 1;
//	submitInfo.pSignalSemaphores = signalSemaphores;
//
//	result = _context.getGraphicsQueue().getQueue().submit(1, &submitInfo, _frames[_currentFrame].inFlightFence);
//
//	vk::PresentInfoKHR presentInfo{};
//	presentInfo.waitSemaphoreCount = 1;
//	presentInfo.pWaitSemaphores = signalSemaphores;
//	presentInfo.swapchainCount = 1;
//	presentInfo.pSwapchains = &_swapChain;
//	presentInfo.pImageIndices = &imageIndex;
//
//	result = _context.getPresentQueue().getQueue().presentKHR(&presentInfo);
//	if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
//	{
//		// Handle window resize (recreate swap chain)
//		handleResize();
//		return;
//	}
//	else if (result != vk::Result::eSuccess)
//	{
//		throw std::runtime_error("Failed to present swap chain image!");
//	}
//
//	_currentFrame = (_currentFrame + 1) % _frames.size();
//}
//
//void VulkanSurface::recordCommandBuffer(FrameData& frame, uint32_t imageIndex)
//{
//	vk::CommandBuffer& commandBuffer = frame.commandBuffer;
//
//	vk::CommandBufferBeginInfo beginInfo{};
//	commandBuffer.begin(beginInfo);
//
////	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline->getPipeline());
////	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipeline->getPipelineLayout(), 0, _descriptorSet, {});
////
////	// render the game surface
////	{
////		vk::ClearValue clearColor = vk::ClearColorValue(std::array<float, 4>{0.5f, 0.5f, 0.5f, 1.0f}); // Gray
////
////		vk::RenderPassBeginInfo renderPassInfo{};
////		renderPassInfo.renderPass = _gameRenderPass;
////		renderPassInfo.framebuffer = _gameFramebuffer;
////		renderPassInfo.renderArea.extent = vk::Extent2D{320, 200};
////		renderPassInfo.clearValueCount = 1;
////		renderPassInfo.pClearValues = &clearColor;
////
////		commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
////
////		// ... any other draw commands for the game content
////		//commandBuffer.draw
////
////		commandBuffer.endRenderPass();
////	}
////
////	// render the window surface
////	{
//		vk::RenderPassBeginInfo renderPassInfo{};
//		renderPassInfo.renderPass = _renderPass;
//		renderPassInfo.framebuffer = frame.framebuffer;
//		renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
//		renderPassInfo.renderArea.extent = _swapChainExtent;
//
//		vk::ClearValue clearColor = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
//		renderPassInfo.clearValueCount = 1;
//		renderPassInfo.pClearValues = &clearColor;
//
//		commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
////
////	    // Bind vertex and index buffers
////		vk::DeviceSize offsets[] = {0};
////		commandBuffer.bindVertexBuffers(0, _vertexBuffer->getBuffer(), offsets);
////		commandBuffer.bindIndexBuffer(_indexBuffer->getBuffer(), 0, vk::IndexType::eUint16);
////
////		// Push up the window transformation matrix
////		commandBuffer.pushConstants(_pipeline->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &_transform);
////
////		// Issue the draw call
//////		commandBuffer.drawIndexed(static_cast<uint32_t>(std::size(indices)), 1, 0, 0, 0);
////
////
//		commandBuffer.endRenderPass();
////	}
////
//	commandBuffer.end();
//
//}

vk::SurfaceKHR VulkanSurface::createSurface(vk::Instance& instance, const PlatformWindowHandle& window)
{
	vk::SurfaceKHR surface;
#if defined(_WIN32)
	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo{};
	surfaceCreateInfo.hwnd = window.hWnd;
	surfaceCreateInfo.hinstance = window.hInstance;

	surface = instance.createWin32SurfaceKHR(surfaceCreateInfo);
	if (!surface)
	{
		throw std::runtime_error("Failed to create Vulkan surface.");
	}
#elif defined(__linux__)
	vk::XlibSurfaceCreateInfoKHR surfaceCreateInfo{};
	surfaceCreateInfo.dpy = (Display*)window.display;
	surfaceCreateInfo.window = window.window;
	surface = instance.createXlibSurfaceKHR(surfaceCreateInfo);
	if (!_surface)
	{
		throw std::runtime_error("Failed to create Vulkan surface.");
	}
#else
#error "Unsupported platform"
#endif

	return surface;
}

void VulkanSurface::destroySurface(vk::Instance& instance, vk::SurfaceKHR& surface)
{
	instance.destroySurfaceKHR(surface);
}


} // namespace OpenXcom
