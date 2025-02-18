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
#include "VulkanSurface.h"
#include "../../GraphicsCommand.h"
#include "../../../Utility/Delegate.h"

#include <vulkan/vulkan.hpp>
#include <glm/vec2.hpp>

namespace OpenXcom
{

class VulkanContext;
class VulkanCommand;
class VulkanBufferFactory;
class VulkanPipelineFactory;
class VulkanPipeline;
class PipelineDefinition;
class PlatformWindow;
class HostBuffer;

struct FrameData
{
	vk::Semaphore imageAvailableSemaphore;
	vk::Semaphore renderFinishedSemaphore;
	vk::Fence inFlightFence;
	vk::ImageView imageView; // For swap chain images
	vk::Framebuffer framebuffer;
	vk::CommandBuffer commandBuffer;
};

// 
class VulkanWindowedSurface : public VulkanSurface
{
protected:
	VulkanContext& _context;

	vk::CommandPool _commandPool;

	vk::SurfaceKHR _surface;
	MulticastDelegate<void(glm::ivec2)>::Handle _onResize;

	vk::SwapchainKHR _swapChain;		
	vk::Format _swapChainImageFormat;
	vk::Extent2D _swapChainExtent;

	std::vector<FrameData> _frames;
	uint32_t _currentFrame;
	uint32_t _imageIndex;
	bool _minimized;

	vk::RenderPass _renderPass;

	PlatformWindow& _window;
	std::unique_ptr<VulkanCommand> _commandContext;
	
	// device image data for a render surface contains the extents of the framebuffer
	std::unique_ptr<HostBuffer> _hostImageData;
	std::unique_ptr<DeviceBuffer> _deviceImageData;

	void initializeSwapChain();
	void destroySwapChain();

	void updateDeviceImageData();
	void handleResize(glm::ivec2 newSize);

	// this function is used to create the surface for the platform window, but equally, VulkanContext needs to
	// create a surface to select a physical device, so rather than duplicate the code, we'll make it static and
	// friendly.
	friend class VulkanContext;
	static vk::SurfaceKHR createSurface(vk::Instance& instance, const PlatformWindow& window);
	static void destroySurface(vk::Instance& instance, vk::SurfaceKHR& surface);

public:
	VulkanWindowedSurface(VulkanContext& context, PlatformWindow& window);
	virtual ~VulkanWindowedSurface();

	const vk::SurfaceKHR& getVKSurface() const { return _surface; }
	const vk::Format& getVKFormat() const { return _swapChainImageFormat; }
	const vk::Extent2D& getVKExtent() const { return _swapChainExtent; }
	const vk::RenderPass& getRenderPass() const { return _renderPass; }

	virtual uint32_t getWidth() const override;
	virtual uint32_t getHeight() const override;
	virtual glm::ivec2 getExtent() const override;
	virtual ImageFormat getFormat() const override;

	virtual uint32_t getMultisampleCount() const override { return 1; }
	virtual bool getUseDynamicStates() const override { return true; }
		
	// device image data for a render surface contains the extents of the framebuffer
	virtual const DeviceBuffer& getDeviceImageData() const override { return *_deviceImageData; }; 

	// copy the host image to this device image
	virtual void copyFrom(HostImage& hostImage) override;

	// copy this device image to the host image
	virtual void copyTo(HostImage& hostImage) override;


	virtual GraphicsCommand& beginCommandPass() override;
	virtual void endCommandPass(GraphicsCommand& commandContext) override;

	virtual void beginRenderPass(GraphicsCommand& commandContext) override;
	virtual void endRenderPass(GraphicsCommand& commandContext) override;
};

} // namespace OpenXcom
