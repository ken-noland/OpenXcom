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
#include "VulkanBuffer.h"
#include "../../Platform/Window.h"

#include "../../Resource/Shader/ShaderManager.h"

#include <glm/glm.hpp>


namespace OpenXcom
{

class VulkanContext;
struct PlatformWindowHandle;
class VulkanBufferFactory;
class VulkanPipelineFactory;
class VulkanPipeline;
class PipelineDefinition;

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

	vk::RenderPass _renderPass;

	//std::unique_ptr<VulkanPipeline> _pipeline;


	//-----
	// the following is temporary(I think, not sure)
	glm::mat4 _transform;

	//descriptor set
	vk::DescriptorSetLayout _descriptorSetLayout;
	vk::DescriptorPool _descriptorPool;
	vk::DescriptorSet _descriptorSet;

//	vk::PipelineLayout _pipelineLayout;
//	vk::Pipeline _pipeline;

	uint32_t _currentFrame;

	PlatformWindowHandle _windowHandle;

	// The main game surface
	vk::DeviceMemory _gameImageMemory;
	vk::Image _gameImage;
	vk::ImageView _gameImageView;
	vk::RenderPass _gameRenderPass;
	vk::Framebuffer _gameFramebuffer;


	//// TEMP
	//ShaderManager::Handle _vertexShader;		// KN Note: when we move to a pipeline registry, this won't be needed here anymore
	//ShaderManager::Handle _fragmentShader;		// KN Note: when we move to a pipeline registry, this won't be needed here anymore

	//// not sure if this'll be needed here once we have resource managers in place to automate the creation and destruction of buffer elements
	//VmaAllocator _allocator;


	vk::Sampler _textureSampler;	// KN Note: this should be moved to a sampler manager/registry of some kind

	std::unique_ptr<VulkanBuffer> _vertexBuffer;
	std::unique_ptr<VulkanBuffer> _indexBuffer;

	//-----

	void initializeSwapChain();
	void initializeRenderPass();
	void initializeFrames();

	//void initializeGameSurface(VulkanBufferFactory& bufferFactory);

	void destroySwapChain();

	void handleResize();

	void recordCommandBuffer(FrameData& frame, uint32_t imageIndex);

	// this function is used to create the surface for the platform window, but equally, VulkanContext needs to
	// create a surface to select a physical device, so rather than duplicate the code, we'll make it static and
	// friendly.
	friend class VulkanContext;
	static vk::SurfaceKHR createSurface(vk::Instance& instance, const PlatformWindowHandle& window);
	static void destroySurface(vk::Instance& instance, vk::SurfaceKHR& surface);

public:
	VulkanSurface(VulkanContext& context, const PlatformWindowHandle& window);
	virtual ~VulkanSurface();

	const vk::SurfaceKHR& getVKSurface() const { return _surface; }
	const vk::Format& getVKFormat() const { return _swapChainImageFormat; }
	const vk::Extent2D& getVKExtent() const { return _swapChainExtent; }

	virtual void draw() override;
};

} // namespace OpenXcom
