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
#include <vulkan/vulkan.hpp>
#include "../GraphicsSurface.h"
#include "../GraphicsCommand.h"

#include "Buffer/VulkanBuffer.h"

#include <glm/glm.hpp>


namespace OpenXcom
{

class VulkanContext;
class VulkanCommand;
class VulkanBufferFactory;
class VulkanPipelineFactory;
class VulkanPipeline;
class PipelineDefinition;
class PlatformWindow;

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
class VulkanSurface : public GraphicsSurface
{
protected:
	VulkanContext& _context;

	vk::CommandPool _commandPool;

	vk::SurfaceKHR _surface;

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

	void initializeSwapChain();
	void destroySwapChain();

	void handleResize();

	// this function is used to create the surface for the platform window, but equally, VulkanContext needs to
	// create a surface to select a physical device, so rather than duplicate the code, we'll make it static and
	// friendly.
	friend class VulkanContext;
	static vk::SurfaceKHR createSurface(vk::Instance& instance, const PlatformWindow& window);
	static void destroySurface(vk::Instance& instance, vk::SurfaceKHR& surface);

public:
	VulkanSurface(VulkanContext& context, PlatformWindow& window);
	virtual ~VulkanSurface();

	const vk::SurfaceKHR& getVKSurface() const { return _surface; }
	const vk::Format& getVKFormat() const { return _swapChainImageFormat; }
	const vk::Extent2D& getVKExtent() const { return _swapChainExtent; }
	const vk::RenderPass& getRenderPass() const { return _renderPass; }

	virtual uint32_t getWidth() const override;
	virtual uint32_t getHeight() const override;

	virtual GraphicsCommand& beginCommandPass() override;
	virtual void endCommandPass(GraphicsCommand& commandContext) override;

	virtual void beginRenderPass(GraphicsCommand& commandContext) override;
	virtual void endRenderPass(GraphicsCommand& commandContext) override;
};

} // namespace OpenXcom
