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
#include "../../Buffer/Buffer.h"

#ifdef __linux__
#undef None // Xlib.h defines None, which conflicts with Vulkan
#endif

#include <simplerttr.h>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h> // VMA

#include <memory>

namespace OpenXcom
{

class VulkanContext;
class VulkanDeviceBuffer;

class VulkanHostBuffer : public HostBuffer
{
protected:
	VulkanContext& _context;
	VmaAllocation _allocation;

	vk::Buffer _buffer;

	void allocate(std::size_t size);
	void deallocate();

public:
	VulkanHostBuffer(VulkanContext& context, VulkanDeviceBuffer& deviceBuffer);
	VulkanHostBuffer(VulkanContext& context, std::size_t elementSize, std::size_t count, BufferUsage usage);
	virtual ~VulkanHostBuffer();

	const vk::Buffer& getBuffer() const { return _buffer; }

	virtual void resize(std::size_t count) override;
	virtual void reserve(std::size_t count) override;
	virtual void clear() override;

	virtual void* map() override;
	virtual void unmap() override;

	virtual void copy(const void* data, std::size_t size) override;
	virtual void copy(DeviceBuffer& deviceBuffer) override;
};

class VulkanDeviceBuffer : public DeviceBuffer
{
private:
	VulkanContext& _context;
	VmaAllocation _allocation;

	vk::Buffer _buffer;

	void allocate(std::size_t size);
	void deallocate();

	void copy(const VulkanHostBuffer& hostBuffer);

public:
	VulkanDeviceBuffer(VulkanContext& context, VulkanHostBuffer& hostBuffer);
	VulkanDeviceBuffer(VulkanContext& context, std::size_t elementSize, std::size_t count, BufferUsage usage); // create a blank device buffer
	virtual ~VulkanDeviceBuffer();

	const vk::Buffer& getBuffer() const { return _buffer; }
		
	virtual void resize(std::size_t count) override;
	virtual void reserve(std::size_t count) override;
	virtual void clear() override;

	virtual void copy(const HostBuffer& buffer) override;
};

} // namespace OpenXcom
