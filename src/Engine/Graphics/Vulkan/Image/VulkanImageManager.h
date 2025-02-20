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

#include "../../Image/ImageManager.h"
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h> // VMA

#include <glm/vec2.hpp>

namespace OpenXcom
{

class VulkanContext;

class VulkanImageManager : public ImageManager
{
protected:
	VulkanContext& _context;

public:
	VulkanImageManager(VulkanContext& context);
	virtual ~VulkanImageManager();

	virtual OwningHandle<HostImage> createHostImage(const std::string& name, glm::ivec2 size, ImageFormat format) override;
	virtual OwningHandle<DeviceImage> createDeviceImage(const std::string& name, glm::ivec2 size, ImageFormat format) override;
	virtual OwningHandle<DeviceImage> createDeviceImage(HostImage& host) override;
};

} // namespace OpenXcom
