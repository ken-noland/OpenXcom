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
#include "BoxPrimitiveFactory.h"

#include "BoxPrimitive.h"
#include "ShaderCollection.h"

#include "../Pipeline.h"
#include "../PipelineDefinition.h"
#include "../PipelineManager.h"

#include "../../EngineContext.h"
#include "../../Resource/ResourceSystem.h"

namespace OpenXcom
{

BoxPrimitiveFactory::BoxPrimitiveFactory(EngineContext& context, RenderTarget& surface, ShaderCollection& shaders)
	: _context(context), _surface(surface)
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	PipelineManager& pipelineManager = resourceSystem.getPipelineManager();

	//---
	// box filled pipeline
	PipelineBuilder boxFilledPipelineBuilder;
	PipelineDefinition boxFilledDefinition = boxFilledPipelineBuilder
												 .setResourceLayout(ResourceLayoutBuilder()
																		// topology
																		.setTopology(PrimitiveTopology::TriangleList)

																		// vertex shader stage
																		.setVertexType<BoxVertex>()
																		.addPushConstant<BoxPushConstants>(ShaderStage::Vertex) // push constant for color
																		.addUniformBuffer<glm::ivec2>(ShaderStage::Vertex, 0)    // screen size buffer
																		.addStorageBuffer<uint8_t>(ShaderStage::Vertex, 1)       // palette buffer

																		.build())
												 .setVertexShader(shaders.getDefaultVec2VertexShader())
												 .setFragmentShader(shaders.getDefaultFragmentShader())
												 .setRenderTarget(_surface)
												 .build();

	_boxFilledPipeline = pipelineManager.createPipeline(boxFilledDefinition);

	//---
	// box outline pipeline
	PipelineBuilder boxOutlinePipelineBuilder;
	PipelineDefinition boxOutlineDefinition = boxOutlinePipelineBuilder
												  .setResourceLayout(ResourceLayoutBuilder()
																		 // topology
																		 .setTopology(PrimitiveTopology::LineStrip) // outline uses lines

																		 // vertex shader stage
																		 .setVertexType<BoxVertex>()
																		 .addPushConstant<BoxPushConstants>(ShaderStage::Vertex) // push constant for color
																		 .addUniformBuffer<glm::ivec2>(ShaderStage::Vertex, 0)   // screen size buffer
																		 .addStorageBuffer<uint8_t>(ShaderStage::Vertex, 1)      // palette buffer

																		 .build())
												  .setVertexShader(shaders.getDefaultVec2VertexShader())
												  .setFragmentShader(shaders.getDefaultFragmentShader())
												  .setRenderTarget(_surface)
												  .build();

	_boxOutlinePipeline = pipelineManager.createPipeline(boxOutlineDefinition);
}

BoxPrimitiveFactory::~BoxPrimitiveFactory()
{
}

std::unique_ptr<BoxFilledPrimitive> BoxPrimitiveFactory::createFilledBoxPrimitive(glm::ivec2 position, glm::ivec2 size, int color, const ResourceHandle<Palette>& paletteHandle)
{
	return std::make_unique<BoxFilledPrimitive>(_context, *_boxFilledPipeline, _surface, position, size, color, paletteHandle);
}

std::unique_ptr<BoxOutlinePrimitive> BoxPrimitiveFactory::createOutlineBoxPrimitive(glm::ivec2 position, glm::ivec2 size, int color, const ResourceHandle<Palette>& palette)
{
	return std::make_unique<BoxOutlinePrimitive>(_context, *_boxOutlinePipeline, _surface, position, size, color, palette);
}

} // namespace OpenXcom
