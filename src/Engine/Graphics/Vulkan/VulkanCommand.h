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
#include "../GraphicsCommand.h"
#include <vulkan/vulkan.hpp>

namespace OpenXcom
{

class VulkanContext;
class RenderTarget;
class Pipeline;

class VulkanCommand : public GraphicsCommand
{
protected:
	VulkanContext& _context;
	RenderTarget* _surface;

	vk::CommandBuffer _commandBuffer;

public:
	VulkanCommand(VulkanContext& context);
	virtual ~VulkanCommand();

	virtual void beginRenderPass(RenderTarget& surface) override;
	virtual void endRenderPass() override;

	virtual void bindPipeline(Pipeline& pipeline) override;


	void setCommandBuffer(const vk::CommandBuffer& commandBuffer) { _commandBuffer = commandBuffer; }
	vk::CommandBuffer& getCommandBuffer() { return _commandBuffer; }
};

} // namespace OpenXcom
