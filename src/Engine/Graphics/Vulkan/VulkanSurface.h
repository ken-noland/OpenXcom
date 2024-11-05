#pragma once
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
#include "../GraphicsSurface.h"
#include "VulkanInclude.h"
#include "../../Platform/Window.h"

namespace OpenXcom
{

struct PlatformWindowHandle;

struct FrameData
{
	vk::Semaphore imageAvailableSemaphore;
	vk::Semaphore renderFinishedSemaphore;
	vk::Fence inFlightFence;
	vk::ImageView imageView; // For swap chain images
	vk::Framebuffer framebuffer;
	vk::CommandBuffer commandBuffer;
};

class VulkanSurface : public GraphicsSurface
{
	vk::Instance _instance;		// reference to instance in VulkanSystem
	vk::Device _device;			// reference to device in VulkanSystem
	vk::PhysicalDevice _physicalDevice; // reference to physical device in VulkanSystem

	vk::Queue _graphicsQueue;	// reference to graphics queue in VulkanSystem
	vk::Queue _presentQueue;	// reference to present queue in VulkanSystem

	vk::CommandPool _commandPool;

	vk::SurfaceKHR _surface;

	vk::SwapchainKHR _swapChain;		
	vk::Format _swapChainImageFormat;
	vk::Extent2D _swapChainExtent;

	std::vector<FrameData> _frames;

	vk::RenderPass _renderPass; // reference to render pass in VulkanSystem

	vk::PipelineLayout _pipelineLayout;
	vk::Pipeline _pipeline;

	uint32_t _currentFrame;

	PlatformWindowHandle _windowHandle;

	// TEMP
	vk::ShaderModule _vertexShaderModule;
	vk::ShaderModule _fragmentShaderModule;

	friend class VulkanSystem;
	void initializeDevice(vk::Device& device, uint32_t graphicsQueueFamilyIndex, vk::Queue& graphicsQueue, vk::Queue& presentQueue);
	void initializeSwapChain(const vk::PhysicalDevice& physicalDevice);
	void initializeFrames(const vk::RenderPass& renderPass);
	void initializeShaders(shaderc::Compiler& compiler);
	void initializePipeline();


	void destroySwapChain();

	void handleResize();

	void recordCommandBuffer(FrameData& frame, uint32_t imageIndex);

public:
	VulkanSurface(vk::Instance instance, const PlatformWindowHandle& window);
	virtual ~VulkanSurface();

	const vk::SurfaceKHR& getVKSurface() const { return _surface; }
	const vk::Format& getVKFormat() const { return _swapChainImageFormat; }
	const vk::Extent2D& getVKExtent() const { return _swapChainExtent; }

	virtual void update() override;
};

} // namespace OpenXcom
