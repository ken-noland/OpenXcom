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
#include "ImagePrimitiveFactory.h"

#include "ImagePrimitive.h"

#include "../Pipeline.h"
#include "../PipelineDefinition.h"
#include "../PipelineManager.h"

#include "../../EngineContext.h"
#include "../../Resource/ResourceSystem.h"

namespace OpenXcom
{


ImagePrimitiveFactory::ImagePrimitiveFactory(EngineContext& context, RenderTarget& surface, ShaderCollection& shaders)
	: _context(context), _surface(surface)
{
	//ResourceSystem& resourceSystem = _context.getResourceSystem();
	//PipelineManager& pipelineManager = resourceSystem.getPipelineManager();
}

ImagePrimitiveFactory::~ImagePrimitiveFactory()
{
}

std::unique_ptr<ImagePrimitive> ImagePrimitiveFactory::createImagePrimitive(glm::ivec2 position, glm::ivec2 size, glm::ivec2 extents, const ResourceHandle<DeviceImage>& imageHandle, const ResourceHandle<Palette>& paletteHandle)
{
	return nullptr;
}


} // namespace OpenXcom
