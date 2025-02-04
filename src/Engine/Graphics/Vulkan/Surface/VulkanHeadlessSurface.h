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
#include "VulkanSurface.h"
#include <memory>

#include <vulkan/vulkan.hpp>
#include <glm/vec2.hpp>

namespace OpenXcom
{

class VulkanContext;
class VulkanCommand;

class VulkanHeadlessSurface : public VulkanSurface
{
protected:
	VulkanContext& _context;

	vk::CommandPool _commandPool;
	vk::CommandBuffer _commandBuffer;

	std::unique_ptr<VulkanCommand> _commandContext;
		
	// device image data for a render surface contains the extents of the framebuffer
	std::unique_ptr<DeviceBuffer> _deviceImageData;

public:
	VulkanHeadlessSurface(VulkanContext& context);
	virtual ~VulkanHeadlessSurface();

	virtual uint32_t getWidth() const override { return 0; }
	virtual uint32_t getHeight() const override { return 0; }

	virtual glm::ivec2 getExtent() const override { return {0, 0}; }
	virtual ImageFormat getFormat() const override { return ImageFormat::UNKNOWN; }

	// device image data for a render surface contains the extents of the framebuffer
	virtual const DeviceBuffer& getDeviceImageData() const override { return *_deviceImageData; }

	virtual void copyFrom(HostImage& hostImage) override;
	virtual void copyTo(HostImage& hostImage) override;

	virtual GraphicsCommand& beginCommandPass() override;
	virtual void endCommandPass(GraphicsCommand& commandContext) override;

	virtual void beginRenderPass(GraphicsCommand& commandContext) override;
	virtual void endRenderPass(GraphicsCommand& commandContext) override;
};

} // namespace OpenXcom
