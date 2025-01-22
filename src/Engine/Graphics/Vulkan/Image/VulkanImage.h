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
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h> // VMA

namespace OpenXcom
{

class VulkanContext;
class VulkanHostImage;
class VulkanRenderTargetImage;

class VulkanImageFactory
{
protected:
	VulkanContext& _context;

public:
	VulkanImageFactory(VulkanContext& context);
	~VulkanImageFactory();

	// create a render target image(note: this is different from a platform window render target image and doesn't contain a swapchain)
	std::unique_ptr<VulkanRenderTargetImage> createRenderTarget(uint32_t width, uint32_t height, ImageFormat format);

	// create an empty host image
	std::unique_ptr<VulkanHostImage> createHostImage(uint32_t width, uint32_t height, ImageFormat format);
};

class VulkanImage : public Image
{
protected:
	VulkanContext& _context;

	VmaAllocation _allocation;

	vk::Image _image;

	int _width;
	int _height;
	ImageFormat _format;

public:
	VulkanImage(VulkanContext& context, uint32_t width, uint32_t height, vk::ImageUsageFlags flags);
	virtual ~VulkanImage();
};

class VulkanHostImage : public VulkanImage
{
};

class VulkanDeviceImage : public VulkanImage
{
};

class VulkanRenderTargetImage : public RenderTargetImage
{
private:
	VulkanContext& _context;

	VmaAllocation _allocation;

	vk::Image _image;
	vk::ImageView _imageView;

	vk::RenderPass _renderPass;
	vk::Framebuffer _framebuffer;

public:
	VulkanRenderTargetImage(VulkanContext& context, uint32_t width, uint32_t height, ImageFormat format);
	virtual ~VulkanRenderTargetImage();
};

} // namespace OpenXcom
