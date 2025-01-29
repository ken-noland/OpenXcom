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
#include "VulkanHeadlessSurface.h"
#include "../VulkanContext.h"
#include "../VulkanCommand.h"

#include <stdexcept>

namespace OpenXcom
{

VulkanHeadlessSurface::VulkanHeadlessSurface(VulkanContext& context)
	: VulkanSurface(VulkanSurfaceType::Headless), _context(context)
{
	// Create a command pool for headless execution
	vk::CommandPoolCreateInfo poolInfo{};
	poolInfo.queueFamilyIndex = _context.getGraphicsQueue().getFamilyIndex();
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

	_commandPool = _context.getDevice().createCommandPool(poolInfo);

	// Allocate a command buffer
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.commandPool = _commandPool;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandBufferCount = 1;

	_commandBuffer = _context.getDevice().allocateCommandBuffers(allocInfo)[0];

	_commandContext = std::make_unique<VulkanCommand>(_context);
	_commandContext->setCommandBuffer(_commandBuffer);
}

VulkanHeadlessSurface::~VulkanHeadlessSurface()
{
	if (_commandPool)
	{
		_context.getDevice().destroyCommandPool(_commandPool);
	}
}

GraphicsCommand& VulkanHeadlessSurface::beginCommandPass()
{
	// Begin recording commands
	vk::CommandBufferBeginInfo beginInfo{};
	_commandBuffer.begin(beginInfo);
	return *_commandContext;
}

void VulkanHeadlessSurface::endCommandPass(GraphicsCommand& commandContext)
{
	// End command buffer recording
	_commandBuffer.end();
}

void VulkanHeadlessSurface::beginRenderPass(GraphicsCommand& commandContext)
{
	throw std::runtime_error("beginRenderPass() should not be called in headless mode.");
}

void VulkanHeadlessSurface::endRenderPass(GraphicsCommand& commandContext)
{
	throw std::runtime_error("endRenderPass() should not be called in headless mode.");
}

} // namespace OpenXcom
