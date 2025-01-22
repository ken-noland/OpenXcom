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
#include <vk_mem_alloc.h> // VMA

namespace OpenXcom
{

class Options;
class VulkanBufferFactory;
class VulkanDescriptorSetFactory;
class VulkanSamplerFactory;
class VulkanPipelineFactory;

class VulkanShaderManager;
class VulkanPipelineManager;

class VulkanQueue
{
protected:
	vk::Queue _queue;
	uint32_t _familyIndex;

	vk::Device _device;

	
	vk::CommandPool _commandPool;
	std::vector<vk::CommandBuffer> _commandBuffers;

	friend class VulkanContext;
	void create(vk::Device device, uint32_t familyIndex, bool shouldCreateCommandBuffer);

public:
	VulkanQueue();
	~VulkanQueue();

	uint32_t getFamilyIndex() { return _familyIndex; }
	vk::Queue& getQueue() { return _queue; }

	vk::CommandPool& getCommandPool() { return _commandPool; }
	vk::CommandBuffer& getCommandBuffer(int index = 0) { return _commandBuffers[index]; }

	bool isValid() { return _familyIndex != std::numeric_limits<uint32_t>::max(); }
	void reset();
};

class VulkanContext
{
protected:
	vk::Instance _instance;
	vk::Device _device;
	vk::PhysicalDevice _physicalDevice;

	vk::DynamicLoader _loader;
	vk::DebugUtilsMessengerEXT _debugMessenger;

	VulkanQueue _graphicsQueue;
	VulkanQueue _transferQueue;
	VulkanQueue _presentQueue;

	vk::Format _swapChainImageFormat;

	VmaAllocator _allocator;

	// the factory for creating vertex, index and other buffers
	std::unique_ptr<VulkanBufferFactory> _bufferFactory;

	// the factory for creating descriptor sets
	std::unique_ptr<VulkanDescriptorSetFactory> _descriptorSetFactory; // not entirely sure this needs to be here, but keeping it here anyway

	// the factory for creating the samplers
	std::unique_ptr<VulkanSamplerFactory> _samplerFactory;

	// the factory for creating pipelines
	std::unique_ptr<VulkanPipelineFactory> _pipelineFactory;

	// pointers ot the vulkan specific managers for the various resources
	VulkanShaderManager* _shaderManager;
	VulkanPipelineManager* _pipelineManager;

	void initializeInstance();
	void selectPhysicalDevice(const vk::SurfaceKHR& surface);
	void initializeDevice(const vk::SurfaceKHR& surface);

public:
	VulkanContext(const Options& options);
	~VulkanContext();

	vk::Instance& getInstance() { return _instance; }
	vk::Device& getDevice() { return _device; }
	vk::PhysicalDevice& getPhysicalDevice() { return _physicalDevice; }

	VmaAllocator& getAllocator() { return _allocator; }

	vk::Format getSwapChainImageFormat() { return _swapChainImageFormat; }

	VulkanBufferFactory& getBufferFactory() { return *_bufferFactory; }
	VulkanDescriptorSetFactory& getDescriptorSetFactory() { return *_descriptorSetFactory; }
	VulkanPipelineFactory& getPipelineFactory() { return *_pipelineFactory; }

	VulkanQueue& getGraphicsQueue() { return _graphicsQueue; }
	VulkanQueue& getTransferQueue() { return _transferQueue; }
	VulkanQueue& getPresentQueue() { return _presentQueue; }
};

} // namespace OpenXcom
