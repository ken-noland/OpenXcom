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
#include "VulkanInclude.h"
#include <iostream>
#include <vector>

namespace OpenXcom
{

class VulkanSurface;

class VulkanSystem : public GraphicsSystem
{
	vk::DynamicLoader _loader;
	vk::Instance _instance;

	vk::DebugUtilsMessengerEXT _debugMessenger;

	vk::PhysicalDevice _physicalDevice;
	vk::Device _device;

	uint32_t _graphicsQueueFamilyIndex;
	uint32_t _presentQueueFamilyIndex;

	vk::Queue _graphicsQueue;
	vk::Queue _presentQueue;

	// we need to store this here so that _renderPass has access to the swap chain format
	vk::Format _swapChainImageFormat;

	vk::RenderPass _renderPass;

	void selectPhysicalDevice(const vk::SurfaceKHR& surface);
	void initializeDevice(const vk::SurfaceKHR& surface);

	void initializeSwapChain(std::unique_ptr<VulkanSurface>& surface, const PlatformWindowHandle& handle);
	void initializeRenderPass();

public:
	VulkanSystem(const Options& options);
	virtual ~VulkanSystem();

	virtual std::unique_ptr<GraphicsSurface> createSurface(const PlatformWindowHandle& handle) override;
};

} // namespace OpenXcom
