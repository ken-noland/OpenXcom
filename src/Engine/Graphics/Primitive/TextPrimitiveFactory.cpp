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
#include "TextPrimitiveFactory.h"

#include "TextPrimitive.h"
#include "ShaderCollection.h"

#include "../Pipeline.h"
#include "../PipelineDefinition.h"
#include "../PipelineManager.h"

#include "../../EngineContext.h"
#include "../../Resource/ResourceSystem.h"

namespace OpenXcom
{

TextPrimitiveFactory::TextPrimitiveFactory(EngineContext& context, RenderTarget& surface, ShaderCollection& shaders)
	: _context(context), _surface(surface), _shaders(shaders)
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	PipelineManager& pipelineManager = resourceSystem.getPipelineManager();

	//---
	// image pipeline
	PipelineBuilder pointListPipelineBuilder;
	PipelineDefinition pointListDefinition = pointListPipelineBuilder
												 .setResourceLayout(ResourceLayoutBuilder()
																		// topology
																		.setTopology(PrimitiveTopology::TriangleList)

																		// vertex shader stage
																		.setVertexType<TextVertex>()
																		.addUniformBuffer<glm::ivec2>(ShaderStage::Vertex, 0) // screen size buffer
																		.addUniformBuffer<glm::ivec2>(ShaderStage::Vertex, 1) // source image size buffer

																		// fragment shader stage
																		.addCombinedImageSampler(ShaderStage::Fragment, 2)   // image sampler
																		.addTexture(ShaderStage::Fragment, 2)                // image texture
																		.addStorageBuffer<uint8_t>(ShaderStage::Fragment, 3) // palette buffer
																		.addPushConstant<TextPushConstant>(ShaderStage::Fragment) // text settings

																		.build())
												 .setVertexShader(shaders.getDefaultVec2UVVertexShader())
												 .setFragmentShader(shaders.getDefaultFontFragmentShader())
												 .setRenderTarget(_surface)
												 .build();

	_textPipeline = pipelineManager.createPipeline(pointListDefinition);
}

TextPrimitiveFactory::~TextPrimitiveFactory()
{
}

std::unique_ptr<TextPrimitive> TextPrimitiveFactory::createTextPrimitive(const std::string& text, const TextSettings& setting)
{
	return std::make_unique<TextPrimitive>(_context, *_textPipeline, _surface, text, setting);
}

} // namespace OpenXcom
