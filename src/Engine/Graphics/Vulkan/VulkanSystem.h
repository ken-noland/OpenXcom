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
#include "../GraphicsSystem.h"

#include "VulkanContext.h"

#include <iostream>
#include <vector>

namespace OpenXcom
{

class VulkanSystem : public GraphicsSystem
{
protected:
	VulkanContext _context;

public:
	VulkanSystem(const Options& options);
	virtual ~VulkanSystem();

	virtual std::unique_ptr<GraphicsSurface> createSurface(const PlatformWindowHandle& handle) override;

	virtual std::unique_ptr<ShaderManager> createShaderManager() override;
	virtual std::unique_ptr<PipelineManager> createPipelineManager() override;
	virtual std::unique_ptr<FontManager> createFontManager() override;
	virtual std::unique_ptr<ImageManager> createImageManager() override;
	virtual std::unique_ptr<PaletteManager> createPaletteManager() override;
};

} // namespace OpenXcom
