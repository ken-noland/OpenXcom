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

namespace OpenXcom
{

struct PlatformWindowHandle;

class VulkanSurface : public GraphicsSurface
{
	vk::Instance _instance;
	vk::SurfaceKHR _surface;
		
	vk::SwapchainKHR _swapChain;
	vk::Device* _device;
		
	vk::Format _swapChainImageFormat;
	vk::Extent2D _swapChainExtent;

	friend class VulkanSystem;
	void initializeSwapChain(const PlatformWindowHandle& handle, const vk::PhysicalDevice& physicalDevice, vk::Device* device);

public:
	VulkanSurface(vk::Instance instance, const PlatformWindowHandle& window);
	virtual ~VulkanSurface();

	const vk::SurfaceKHR& getVKSurface() const { return _surface; }
	const vk::Format& getVKFormat() const { return _swapChainImageFormat; }
	const vk::Extent2D& getVKExtent() const { return _swapChainExtent; }
};

} // namespace OpenXcom
