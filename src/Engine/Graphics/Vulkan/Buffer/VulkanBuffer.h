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
#include "../../Buffer.h"

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h> // VMA
#include <simplerttr.h>

#include <memory>

namespace OpenXcom
{

class VulkanContext;

class VulkanBuffer
{
protected:
	VulkanContext& _context;
	VmaAllocation _allocation;

	SimpleRTTR::TypeReference _type;

	vk::Buffer _buffer;
	vk::DeviceSize _size;

	static vk::BufferUsageFlags getUsageFlags(BufferUsage usage);

public:
	VulkanBuffer(VulkanContext& context, const SimpleRTTR::Type& type, vk::DeviceSize size) : _context(context), _allocation(0), _type(type), _buffer(), _size(size) {}
	virtual ~VulkanBuffer() = default;

	vk::DeviceSize getSize() const { return _size; }
	vk::Buffer getBuffer() const { return _buffer; }

	SimpleRTTR::Type getType() { return _type.type(); }
};

class VulkanHostBuffer : public VulkanBuffer, public HostBuffer
{
private:

public:
	VulkanHostBuffer(VulkanContext& context, const SimpleRTTR::Type& type, vk::DeviceSize size, BufferUsage usage);
	virtual ~VulkanHostBuffer();

	void* map();
	void unmap();

	virtual void copyTo(const void* data, size_t offset, size_t size) override;
};

class VulkanDeviceBuffer : public VulkanBuffer, public DeviceBuffer
{
private:
	void create();
	void update(VulkanHostBuffer& hostBuffer);

public:
	VulkanDeviceBuffer(VulkanContext& context, VulkanHostBuffer& hostBuffer);
	VulkanDeviceBuffer(VulkanContext& context, const SimpleRTTR::Type& type, vk::DeviceSize size, BufferUsage usage); // create a blank device buffer
	virtual ~VulkanDeviceBuffer();
};




} // namespace OpenXcom
