
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
#include "VulkanBufferManager.h"
#include "VulkanBuffer.h"
#include "../VulkanContext.h"


namespace OpenXcom
{

VulkanBufferManager::VulkanBufferManager(VulkanContext& context)
	: _context(context)
{
}

VulkanBufferManager::~VulkanBufferManager()
{
}

std::unique_ptr<HostBuffer> VulkanBufferManager::createHostBuffer(std::size_t elementSize, std::size_t count, BufferUsage usage)
{
	return std::make_unique<VulkanHostBuffer>(_context, elementSize, count, usage);
}

std::unique_ptr<HostBuffer> VulkanBufferManager::createHostBuffer(DeviceBuffer& deviceBuffer)
{
	return std::make_unique<VulkanHostBuffer>(_context, static_cast<VulkanDeviceBuffer&>(deviceBuffer));
}

std::unique_ptr<DeviceBuffer> VulkanBufferManager::createDeviceBuffer(std::size_t elementSize, std::size_t count, BufferUsage usage)
{
	return std::make_unique<VulkanDeviceBuffer>(_context, elementSize, count, usage);
}

std::unique_ptr<DeviceBuffer> VulkanBufferManager::createDeviceBuffer(HostBuffer& hostBuffer)
{
	VulkanHostBuffer& vkHostBuffer = static_cast<VulkanHostBuffer&>(hostBuffer);
	return std::make_unique<VulkanDeviceBuffer>(_context, vkHostBuffer);
}

} // namespace OpenXcom
