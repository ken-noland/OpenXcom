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
#include "VulkanSurface.h"

#include "Font/VulkanFontManager.h"
#include "Image/VulkanImageManager.h"
#include "Palette/VulkanPaletteManager.h"
#include "Shader/VulkanShaderManager.h"
#include "Pipeline/VulkanPipelineManager.h"





VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

namespace OpenXcom
{



VulkanSystem::VulkanSystem(const Options& options)
	: _context(options)
{
}

VulkanSystem::~VulkanSystem()
{

}

std::unique_ptr<GraphicsSurface> VulkanSystem::createSurface(const PlatformWindowHandle& handle)
{
	return std::make_unique<VulkanSurface>(_context, handle);
}

std::unique_ptr<ShaderManager> VulkanSystem::createShaderManager()
{
	return std::make_unique<VulkanShaderManager>(_context);
}

std::unique_ptr<PipelineManager> VulkanSystem::createPipelineManager()
{
	return std::make_unique<VulkanPipelineManager>(_context);
}

std::unique_ptr<FontManager> VulkanSystem::createFontManager()
{
	return std::make_unique<VulkanFontManager>(_context);
}

std::unique_ptr<ImageManager> VulkanSystem::createImageManager()
{
	return std::make_unique<VulkanImageManager>(_context);
}

std::unique_ptr<PaletteManager> VulkanSystem::createPaletteManager()
{
	return std::make_unique<VulkanPaletteManager>(_context);
}


} // namespace OpenXcom
