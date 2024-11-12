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
#include "../GraphicsSystem.h"

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h> // VMA

#include <iostream>
#include <vector>

namespace OpenXcom
{

class VulkanSurface;
class VulkanShaderManager;
class VulkanBufferFactory;

class VulkanSystem : public GraphicsSystem
{
	vk::DynamicLoader _loader;
	vk::Instance _instance;

	vk::DebugUtilsMessengerEXT _debugMessenger;

	vk::PhysicalDevice _physicalDevice;
	vk::Device _device;

	VmaAllocator _allocator;

	uint32_t _graphicsQueueFamilyIndex;
	uint32_t _presentQueueFamilyIndex;
	uint32_t _transferQueueFamilyIndex;

	vk::Queue _graphicsQueue;
	vk::Queue _presentQueue;

	// we need to store this here so that _renderPass has access to the swap chain format
	vk::Format _swapChainImageFormat;

	vk::RenderPass _renderPass;

	// the factory for creating buffers
	std::unique_ptr<VulkanBufferFactory> _bufferFactory;

	// hold on to device specific manager pointers for cleanup
	VulkanShaderManager* _shaderManager;

	void selectPhysicalDevice(const vk::SurfaceKHR& surface);
	void initializeDevice(const vk::SurfaceKHR& surface);

	void initializeSwapChain(std::unique_ptr<VulkanSurface>& surface);
	void initializeRenderPass();
	void initializeFrames(std::unique_ptr<VulkanSurface>& surface);

public:
	VulkanSystem(const Options& options);
	virtual ~VulkanSystem();

	virtual std::unique_ptr<GraphicsSurface> createSurface(const PlatformWindowHandle& handle) override;

	virtual std::unique_ptr<FontManager> createFontManager() override;
	virtual std::unique_ptr<PaletteManager> createPaletteManager() override;
	virtual std::unique_ptr<ShaderManager> createShaderManager() override;
};

} // namespace OpenXcom
