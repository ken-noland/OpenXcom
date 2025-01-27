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

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h> // VMA


namespace OpenXcom
{

enum class ImageFormat;
class VulkanContext;

class VulkanRenderTarget : public RenderTarget
{
private:
	VulkanContext& _context;

	VmaAllocation _allocation;

	uint32_t _width;
	uint32_t _height;

	vk::Image _image;
	vk::ImageView _imageView;

	vk::RenderPass _renderPass;
	vk::Framebuffer _framebuffer;

public:
	VulkanRenderTarget(VulkanContext& context, uint32_t width, uint32_t height, ImageFormat format);
	virtual ~VulkanRenderTarget();

	virtual uint32_t getWidth() const override { return _width; }
	virtual uint32_t getHeight() const override { return _height; }

	vk::Image& getImage() { return _image; }
	const vk::ImageView& getImageView() const { return _imageView; }

	virtual void beginRenderPass(GraphicsCommand& commandContext) override;
	virtual void endRenderPass(GraphicsCommand& commandContext) override;
};

} // namespace OpenXcom
