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
#include <memory>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h> // VMA
#include <optional>

#include "VulkanQueue.h"

namespace OpenXcom
{

class EngineContext;
class Options;
class VulkanBufferFactory;
class VulkanDescriptorSetFactory;
class VulkanSamplerFactory;
class VulkanPipelineFactory;

class VulkanShaderManager;
class VulkanPipelineManager;

enum class BufferUsage;

class VulkanContext
{
protected:
	EngineContext& _engineContext;

	vk::Instance _instance;
	vk::Device _device;
	vk::PhysicalDevice _physicalDevice;

	vk::DynamicLoader _loader;
	vk::DebugUtilsMessengerEXT _debugMessenger;

	VulkanQueue _graphicsQueue;
	VulkanQueue _transferQueue;
	VulkanQueue _presentQueue;

	VulkanQueueThread _queueThread;

	vk::Format _swapChainImageFormat;

	VmaAllocator _allocator;

	// the factory for creating descriptor sets
	std::unique_ptr<VulkanDescriptorSetFactory> _descriptorSetFactory; // not entirely sure this needs to be here, but keeping it here anyway

	// the factory for creating the samplers
	std::unique_ptr<VulkanSamplerFactory> _samplerFactory;

	// the factory for creating pipelines
	std::unique_ptr<VulkanPipelineFactory> _pipelineFactory;

	// pointers ot the vulkan specific managers for the various resources
	VulkanShaderManager* _shaderManager;
	VulkanPipelineManager* _pipelineManager;

	void initializeInstance(bool isHeadless);
	void selectPhysicalDevice(std::optional<vk::SurfaceKHR> surface);
	void initializeDevice(std::optional<vk::SurfaceKHR> surface);

	bool checkPhysicalDeviceHasFeatures(const vk::PhysicalDeviceProperties& properties, const vk::PhysicalDeviceFeatures& features);

public:
	VulkanContext(EngineContext& context);
	~VulkanContext();

	EngineContext& getEngineContext() { return _engineContext; }

	vk::Instance& getInstance() { return _instance; }
	vk::Device& getDevice() { return _device; }
	vk::PhysicalDevice& getPhysicalDevice() { return _physicalDevice; }

	VmaAllocator& getAllocator() { return _allocator; }

	vk::Format getSwapChainImageFormat() { return _swapChainImageFormat; }

	VulkanDescriptorSetFactory& getDescriptorSetFactory() { return *_descriptorSetFactory; }
	VulkanSamplerFactory& getSamplerFactory() { return *_samplerFactory; }
	VulkanPipelineFactory& getPipelineFactory() { return *_pipelineFactory; }

	VulkanQueue& getGraphicsQueue() { return _graphicsQueue; }
	VulkanQueue& getTransferQueue() { return _transferQueue; }
	VulkanQueue& getPresentQueue() { return _presentQueue; }

	VulkanQueueThread& getGraphicsQueueThread() { return _queueThread; }
	VulkanQueueThread& getTransferQueueThread() { return _queueThread; }

	// helpers
	vk::BufferUsageFlags getBufferUsageFlags(BufferUsage usage);

	// settings
};

} // namespace OpenXcom
