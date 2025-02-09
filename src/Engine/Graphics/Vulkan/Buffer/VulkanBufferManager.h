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

#include "../../Buffer/BufferManager.h"
#include <vulkan/vulkan.hpp>
#include <memory>


namespace OpenXcom
{

class VulkanContext;
class VulkanHostBuffer;
class VulkanDeviceBuffer;

class VulkanBufferManager : public BufferManager
{
private:
	VulkanContext& _context;

public:
	VulkanBufferManager(VulkanContext& context);
	~VulkanBufferManager();

	// Create an empty host buffer
	virtual std::unique_ptr<HostBuffer> createHostBuffer(std::size_t elementSize, std::size_t count, BufferUsage usage) override;

	// Create a host buffer from a device buffer
	virtual std::unique_ptr<HostBuffer> createHostBuffer(DeviceBuffer& deviceBuffer) override;

	// Create an empty device buffer
	virtual std::unique_ptr<DeviceBuffer> createDeviceBuffer(std::size_t elementSize, std::size_t count, BufferUsage usage) override;

	// Create a device buffer from a host buffer
	virtual std::unique_ptr<DeviceBuffer> createDeviceBuffer(HostBuffer& hostBuffer) override;
};

} // namespace OpenXcom
