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
#include "VulkanCommand.h"
#include "VulkanSurface.h"
#include "Pipeline/VulkanPipeline.h"

namespace OpenXcom
{

VulkanCommand::VulkanCommand(VulkanContext& context)
	: _context(context), _surface(nullptr)
{
}

VulkanCommand::~VulkanCommand()
{
}

void VulkanCommand::beginRenderPass(RenderTarget& surface)
{
	assert(_surface == nullptr);

	_surface = &surface;
	_surface->beginRenderPass(*this);
}

void VulkanCommand::endRenderPass()
{
	assert(_surface != nullptr);

	_surface->endRenderPass(*this);
	_surface = nullptr;
}

void VulkanCommand::bindPipeline(Pipeline& pipeline)
{
	VulkanPipeline& vulkanPipeline = static_cast<VulkanPipeline&>(pipeline);
}

} // namespace OpenXcom
