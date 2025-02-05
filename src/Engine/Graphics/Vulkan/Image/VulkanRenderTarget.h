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
#include "../../GraphicsSurface.h"

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h> // VMA

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>


namespace OpenXcom
{

enum class ImageFormat;
class VulkanContext;

class VulkanRenderTarget : public RenderTarget
{
private:
	VulkanContext& _context;

	std::array<vk::Image, 2> _image;
	std::array<vk::ImageView, 2> _imageView;
	std::array<VmaAllocation, 2> _allocation;

	glm::ivec2 _extent;
	glm::vec4 _color;

	vk::RenderPass _renderPass;
	vk::Framebuffer _framebuffer;

	std::unique_ptr<DeviceBuffer> _deviceImageData;

	vk::SampleCountFlagBits getSampleCountFlagBits() { return static_cast<vk::SampleCountFlagBits>(getMultisampleCount()); }

	void createMultiSampleImage();
	void createSingleSampleImage();

	void create();
	void destroy();

public:
	VulkanRenderTarget(VulkanContext& context, glm::ivec2 size, ImageFormat format, glm::vec4 color);
	virtual ~VulkanRenderTarget();

	virtual uint32_t getWidth() const override { return _extent.x; }
	virtual uint32_t getHeight() const override { return _extent.y; }
	virtual glm::ivec2 getExtent() const override { return _extent; }
	virtual ImageFormat getFormat() const override { return ImageFormat::UNKNOWN; }
	virtual uint32_t getMultisampleCount() const override { return 2; }
		
	virtual void copyFrom(HostImage& hostImage) override;
	virtual void copyTo(HostImage& hostImage) override;

	// device buffer for a render target contains the extents of the render target
	virtual const DeviceBuffer& getDeviceImageData() const override { return *_deviceImageData; }

	const vk::Image& getRenderTargetImage() const { return _image[0]; }
	const vk::ImageView& getRenderTargetImageView() const { return _imageView[0]; }

	const vk::Image& getResolveImage() const { return _image[1]; }
	const vk::ImageView& getResolveImageView() const { return _imageView[1]; }

	const vk::RenderPass& getRenderPass() const { return _renderPass; }

	virtual void beginRenderPass(GraphicsCommand& commandContext) override;
	virtual void endRenderPass(GraphicsCommand& commandContext) override;
};

} // namespace OpenXcom
