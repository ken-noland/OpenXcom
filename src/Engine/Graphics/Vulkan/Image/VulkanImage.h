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

#include "../../../Resource/Image/Image.h"
#include <memory>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h> // VMA

namespace OpenXcom
{

class VulkanContext;

class VulkanHostImage : public HostImage
{
protected:
	VulkanContext& _context;

	VmaAllocation _allocation;

	vk::Buffer _buffer;

	glm::ivec2 _extent;
	ImageFormat _format;

public:
	VulkanHostImage(VulkanContext& context, glm::vec2 size, ImageFormat format);
	virtual ~VulkanHostImage();

	virtual ImageFormat getFormat() const override;
	virtual glm::ivec2 getExtent() const override;
	virtual uint32_t getWidth() const override;
	virtual uint32_t getHeight() const override;

	virtual void* map() override;
	virtual void unmap() override;

	const vk::Buffer& getBuffer() const { return _buffer; }
};

class VulkanDeviceImage : public DeviceImage
{
protected:
	VulkanContext& _context;

	VmaAllocation _allocation;

	vk::Image _image;
	vk::ImageView _imageView;

	vk::ImageLayout _currentLayout;

	glm::ivec2 _extent;
	ImageFormat _format;

public:
	VulkanDeviceImage(VulkanContext& context, glm::vec2 size, ImageFormat format);
	VulkanDeviceImage(VulkanContext& context, VulkanHostImage& image);
	virtual ~VulkanDeviceImage();

	virtual void copyFrom(HostImage& hostImage) override;
	virtual void copyTo(HostImage& hostImage) override;

	virtual ImageFormat getFormat() const override;
	virtual glm::ivec2 getExtent() const override;
	virtual uint32_t getWidth() const override;
	virtual uint32_t getHeight() const override;

	const vk::Image& getImage() const { return _image; }
	const vk::ImageView& getImageView() const { return _imageView; }
};

} // namespace OpenXcom
