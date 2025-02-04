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
#include "LinePrimitiveFactory.h"

#include "LinePrimitive.h"
#include "ShaderCollection.h"

#include "../Pipeline.h"
#include "../PipelineDefinition.h"
#include "../PipelineManager.h"

#include "../../EngineContext.h"
#include "../../Resource/ResourceSystem.h"

namespace OpenXcom
{



LinePrimitiveFactory::LinePrimitiveFactory(EngineContext& context, RenderTarget& surface, ShaderCollection& shaders)
	: _context(context), _surface(surface)
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	PipelineManager& pipelineManager = resourceSystem.getPipelineManager();

	//---
	// line list pipeline
	PipelineBuilder lineListPipelineBuilder;
	PipelineDefinition lineListDefinition = lineListPipelineBuilder
												.setResourceLayout(ResourceLayoutBuilder()
																	   // topology
																	   .setTopology(PrimitiveTopology::LineList)

																	   // vertex shader stage
																	   .setVertexType<LineVertex>()
																	   .addPushConstant<LinePushConstants>(ShaderStage::Vertex) // push constant for color
																	   .addUniformBuffer<glm::ivec2>(ShaderStage::Vertex, 0)    // screen size buffer
																	   .addStorageBuffer<uint8_t>(ShaderStage::Vertex, 1)       // palette buffer

																	   .build())
												.setVertexShader(shaders.getDefaultVec2VertexShader())
												.setFragmentShader(shaders.getDefaultFragmentShader())
												.setRenderTarget(_surface)
												.build();

	_lineListPipeline = pipelineManager.createPipeline(lineListDefinition);

	//---
	// line strip pipeline
	PipelineBuilder lineStripPipelineBuilder;
	PipelineDefinition lineStripDefinition = lineStripPipelineBuilder
												 .setResourceLayout(ResourceLayoutBuilder()
																		// topology
																		.setTopology(PrimitiveTopology::LineStrip)

																		// vertex shader stage
																		.setVertexType<LineVertex>()
																		.addPushConstant<LinePushConstants>(ShaderStage::Vertex) // push constant for color
																		.addUniformBuffer<glm::ivec2>(ShaderStage::Vertex, 0)    // screen size buffer
																		.addStorageBuffer<uint8_t>(ShaderStage::Vertex, 1)       // palette buffer

																		.build())
												 .setVertexShader(shaders.getDefaultVec2VertexShader())
												 .setFragmentShader(shaders.getDefaultFragmentShader())
												 .setRenderTarget(_surface)
												 .build();

	_lineStripPipeline = pipelineManager.createPipeline(lineStripDefinition);
}

LinePrimitiveFactory::~LinePrimitiveFactory()
{
}

std::unique_ptr<LineListPrimitive> LinePrimitiveFactory::createLineListPrimitive(const LineVertex* lines, size_t count, int color, const ResourceHandle<Palette>& paletteHandle)
{
	return std::make_unique<LineListPrimitive>(_context, *_lineListPipeline, _surface, lines, count, color, paletteHandle);
}

std::unique_ptr<LineStripPrimitive> LinePrimitiveFactory::createLineStripPrimitive(const LineVertex* lines, size_t count, int color, const ResourceHandle<Palette>& paletteHandle)
{
	return std::make_unique<LineStripPrimitive>(_context, *_lineStripPipeline, _surface, lines, count, color, paletteHandle);
}

} // namespace OpenXcom
