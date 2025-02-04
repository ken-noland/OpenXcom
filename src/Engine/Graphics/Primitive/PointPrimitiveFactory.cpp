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
#include "PointPrimitiveFactory.h"

#include "PointPrimitive.h"
#include "ShaderCollection.h"

#include "../Pipeline.h"
#include "../PipelineDefinition.h"
#include "../PipelineManager.h"

#include "../../EngineContext.h"
#include "../../Resource/ResourceSystem.h"

namespace OpenXcom
{

PointPrimitiveFactory::PointPrimitiveFactory(EngineContext& context, RenderTarget& surface, ShaderCollection& shaders)
	: _context(context), _surface(surface), _shaders(shaders)
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	PipelineManager& pipelineManager = resourceSystem.getPipelineManager();

	//---
	// point list pipeline
	PipelineBuilder pointListPipelineBuilder;
	PipelineDefinition pointListDefinition = pointListPipelineBuilder
												 .setResourceLayout(ResourceLayoutBuilder()
																		// topology
																		.setTopology(PrimitiveTopology::PointList)

																		// vertex shader stage
																		.setVertexType<PointVertex>()
																		.addPushConstant<PointPushConstants>(ShaderStage::Vertex) // push constant for color
																		.addUniformBuffer<glm::ivec2>(ShaderStage::Vertex, 0) // screen size buffer
																		.addStorageBuffer<uint8_t>(ShaderStage::Vertex, 1)    // palette buffer

																		.build())
												 .setVertexShader(shaders.getDefaultVec2VertexShader())
												 .setFragmentShader(shaders.getDefaultFragmentShader())
												 .setRenderTarget(_surface)
												 .build();

	_pointListPipeline = pipelineManager.createPipeline(pointListDefinition);

	
	//---
	// point color list pipeline
	PipelineBuilder pointColorListPipelineBuilder;
	PipelineDefinition pointColorListDefinition = pointColorListPipelineBuilder
													  .setResourceLayout(ResourceLayoutBuilder()
																			 // topology
																			 .setTopology(PrimitiveTopology::PointList)

																			 // vertex shader stage
																			 .setVertexType<PointColorVertex>()
																			 .addUniformBuffer<glm::ivec2>(ShaderStage::Vertex, 0) // screen size buffer
																			 .addStorageBuffer<uint8_t>(ShaderStage::Vertex, 1)    // palette buffer

																			 .build())
													  .setVertexShader(shaders.getDefaultVec2ColorVertexShader())
													  .setFragmentShader(shaders.getDefaultFragmentShader())
													  .setRenderTarget(_surface)
													  .build();

	_pointColorListPipeline = pipelineManager.createPipeline(pointColorListDefinition);
}

PointPrimitiveFactory::~PointPrimitiveFactory()
{
}

std::unique_ptr<PointListPrimitive> PointPrimitiveFactory::createPointListPrimitive(const PointVertex* points, size_t count, int color, const ResourceHandle<Palette>& paletteHandle)
{
	return std::make_unique<PointListPrimitive>(_context, *_pointListPipeline, _surface, points, count, color, paletteHandle);
}

std::unique_ptr<PointColorListPrimitive> PointPrimitiveFactory::createPointColorListPrimitive(const PointColorVertex* points, size_t count, const ResourceHandle<Palette>& paletteHandle)
{
	return std::make_unique<PointColorListPrimitive>(_context, *_pointColorListPipeline, _surface, points, count, paletteHandle);
}

} // namespace OpenXcom
