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

#include "ResourceSystem.h"

#include "../EngineContext.h"
#include "../Graphics/GraphicsSystem.h"
#include "../Graphics/ShaderManager.h"
#include "../Graphics/PipelineManager.h"
#include "../Graphics/BufferManager.h"
#include "../Graphics/Palette/PaletteManager.h"
#include "../Graphics/Image/ImageManager.h"

#include "Font/FontManager.h"

namespace OpenXcom
{

ResourceSystem::ResourceSystem(EngineContext& context)
{
	GraphicsSystem& graphicsSystem = context.getGraphicsSystem();

	_shaderManager = graphicsSystem.createShaderManager();
	_pipelineManager = graphicsSystem.createPipelineManager();
	_bufferManager = graphicsSystem.createBufferManager();

	_paletteManager = std::make_unique<PaletteManager>(context);
	_fontManager = std::make_unique<FontManager>();
	_imageManager = graphicsSystem.createImageManager();
}

ResourceSystem::~ResourceSystem()
{
}

} // namespace OpenXcom
