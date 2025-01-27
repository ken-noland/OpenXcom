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

#include "../../Utility/RTTR.h"


namespace OpenXcom
{

//vk::Extent2D getClientAreaSize(const PlatformWindow& handle);

VulkanSurface::VulkanSurface(VulkanContext& context, PlatformWindow& window)
	: _context(context), _surface(nullptr), _swapChain(nullptr), _swapChainImageFormat(vk::Format::eUndefined), _swapChainExtent{},
	  _frames(), _currentFrame(0), _imageIndex(0), _minimized(false), _renderPass(nullptr), _window(window)
{
	_surface = createSurface(_context.getInstance(), window);

	initializeSwapChain();

	//register a callback with the window for resize events
	window.onResize() << std::bind(&VulkanSurface::handleResize, this);

	_commandContext = std::make_unique<VulkanCommand>(_context);
}

VulkanSurface::~VulkanSurface()
{
	// Wait for the device to finish
	_context.getDevice().waitIdle();

	destroySwapChain();

	if (_surface)
	{
		destroySurface(_context.getInstance(), _surface);
		_surface = nullptr;
	}
}


void VulkanSurface::initializeSwapChain()
{
	// Get the surface capabilities, formats, and present modes
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = _context.getPhysicalDevice().getSurfaceCapabilitiesKHR(_surface);

	// Check if the window is minimized
	vk::Extent2D clientSize = surfaceCapabilities.currentExtent;
	if (clientSize.width == 0 || clientSize.height == 0)
	{
		// don't need to do anything more here, we'll just wait for the window to be restored
		_minimized = true;
		return;
	}
	_minimized = false;

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
		swapChainExtent = surfaceCapabilities.currentExtent;
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

void VulkanSurface::destroySwapChain()
{
	// clear frames
	for (const FrameData& frame : _frames)
	{
		frame.commandBuffer.reset();
		_context.getDevice().destroyImageView(frame.imageView);
		_context.getDevice().destroyFramebuffer(frame.framebuffer);
		_context.getDevice().destroySemaphore(frame.imageAvailableSemaphore);
		_context.getDevice().destroySemaphore(frame.renderFinishedSemaphore);
		_context.getDevice().destroyFence(frame.inFlightFence);
		_context.getDevice().freeCommandBuffers(_commandPool, 1, &frame.commandBuffer);
	}
	_frames.clear();

	// reset the command pool
	_context.getDevice().destroyCommandPool(_commandPool);

	// destroy the render pass
	if (_renderPass)
	{
		_context.getDevice().destroyRenderPass(_renderPass);
		_renderPass = nullptr;
	}

	// destroy the swap chain
	if (_swapChain)
	{
		_context.getDevice().destroySwapchainKHR(_swapChain);
		_swapChain = nullptr;
	}

	_currentFrame = 0;
	_imageIndex = 0;
}

void VulkanSurface::handleResize()
{
	// Wait for the device to finish
	_context.getDevice().waitIdle();

	// Destroy the current swap chain
	destroySwapChain();

	// Recreate the swap chain
	initializeSwapChain();
}

vk::SurfaceKHR VulkanSurface::createSurface(vk::Instance& instance, const PlatformWindow& window)
{
	vk::SurfaceKHR surface;
#if defined(_WIN32)
	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo{};
	surfaceCreateInfo.hwnd = window.getHandle().hWnd;
	surfaceCreateInfo.hinstance = window.getHandle().hInstance;

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
	if (!surface)
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

uint32_t VulkanSurface::getWidth() const
{
	return _swapChainExtent.width;
}

uint32_t VulkanSurface::getHeight() const
{
	return _swapChainExtent.height;
}

glm::ivec2 VulkanSurface::getSize() const
{
	return glm::ivec2(_swapChainExtent.width, _swapChainExtent.height);
}

GraphicsCommand& VulkanSurface::beginCommandPass()
{
	// assert if the window is minimized(caller needs to check before calling this function)
	assert(!_minimized);

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
	vk::Viewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(_swapChainExtent.width);
	viewport.height = static_cast<float>(_swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	_commandContext->getCommandBuffer().setViewport(0, 1, &viewport);

	vk::Rect2D scissor = {};
	scissor.offset = vk::Offset2D{0, 0};
	scissor.extent = _swapChainExtent;

	_commandContext->getCommandBuffer().setScissor(0, 1, &scissor);

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

} // namespace OpenXcom
