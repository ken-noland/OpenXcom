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

#include "VulkanSystem.h"
#include "Surface/VulkanWindowedSurface.h"
#include "Surface/VulkanHeadlessSurface.h"
#include "Image/VulkanRenderTarget.h"

#include "../../EngineContext.h"
#include "../../Options.h"

#include "Image/VulkanImageManager.h"
#include "Shader/VulkanShaderManager.h"
#include "Pipeline/VulkanPipelineManager.h"
#include "Buffer/VulkanBufferManager.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

namespace OpenXcom
{



VulkanSystem::VulkanSystem(EngineContext& context)
	: _context(context)
{
}

VulkanSystem::~VulkanSystem()
{

}

std::unique_ptr<GraphicsSurface> VulkanSystem::createWindowedSurface(PlatformWindow& window)
{
	std::unique_ptr<GraphicsSurface> surface;
	surface = std::make_unique<VulkanWindowedSurface>(_context, window);
	return surface;
}

std::unique_ptr<GraphicsSurface> VulkanSystem::createHeadlessSurface()
{
	std::unique_ptr<GraphicsSurface> surface;
	surface = std::make_unique<VulkanHeadlessSurface>(_context);
	return surface;
}

std::unique_ptr<RenderTarget> VulkanSystem::createRenderTarget(glm::ivec2 size, ImageFormat format, glm::vec4 color)
{
	return std::make_unique<VulkanRenderTarget>(_context, size, format, color);
}

std::unique_ptr<ShaderManager> VulkanSystem::createShaderManager()
{
	return std::make_unique<VulkanShaderManager>(_context);
}

std::unique_ptr<PipelineManager> VulkanSystem::createPipelineManager()
{
	return std::make_unique<VulkanPipelineManager>(_context);
}

std::unique_ptr<BufferManager> VulkanSystem::createBufferManager()
{
	return std::make_unique<VulkanBufferManager>(_context);
}

std::unique_ptr<ImageManager> VulkanSystem::createImageManager()
{
	return std::make_unique<VulkanImageManager>(_context);
}

} // namespace OpenXcom
