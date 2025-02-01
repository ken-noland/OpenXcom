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

#include "../Pipeline.h"
#include "../PipelineDefinition.h"
#include "../PipelineManager.h"
#include "../Shader.h"
#include "../ShaderManager.h"

#include "../../EngineContext.h"
#include "../../Resource/ResourceSystem.h"

namespace OpenXcom
{

// Vertex shader used to render the game surface to the platform window screen
const char* defaultVertexBoxDrawShaderSource = R"(
	#version 450

	layout(location = 0) in ivec2 inPosition;  // Input as signed integers

	layout(location = 0) out vec4 fragColor;  // Output color

	layout(push_constant) uniform PushConstants {
		int screenWidth;    //TODO: Move to uniform buffer
		int screenHeight;   //TODO: Move to uniform buffer
		int paletteColor;
	} pushConstants;

	layout(set = 0, binding = 0, std430) buffer Palette {
		uint colors[];     // Dynamically sized palette
	};

	void main() {
		// Convert screen coordinates to normalized device coordinates (NDC)
		ivec2 snappedPosition = inPosition; // Already in integer format
		vec2 ndc = (((vec2(snappedPosition) + vec2(0.5, 0.5)) / vec2(pushConstants.screenWidth, pushConstants.screenHeight)) * 2.0) - 1.0;

		gl_Position = vec4(ndc, 0.0, 1.0);

		uint packedColor = colors[pushConstants.paletteColor];

		fragColor = vec4(
			float((packedColor >> 0) & 0xFF) / 255.0,   // Red
			float((packedColor >> 8) & 0xFF) / 255.0,   // Green
			float((packedColor >> 16) & 0xFF) / 255.0,  // Blue
			float((packedColor >> 24) & 0xFF) / 255.0   // Alpha
		);
	}
)";

// Fragment shader used to render the game surface to the platform window screen
const char* defaultFragmentBoxDrawShaderSource = R"(
	#version 450

	// Input from the vertex shader
	layout(location = 0) in vec4 fragColor; // Interpolated color from the vertex shader

	// Output to the framebuffer
	layout(location = 0) out vec4 outColor;

	void main() {
		// Write the interpolated color to the output
		outColor = fragColor;
	}
)";

BoxPrimitiveFactory::BoxPrimitiveFactory(EngineContext& context, RenderTarget& surface)
	: _context(context), _surface(surface)
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	ShaderManager& shaderManager = resourceSystem.getShaderManager();
	PipelineManager& pipelineManager = resourceSystem.getPipelineManager();

	// load the shaders
	_vertexShader = shaderManager.loadShaderFromMemory("BoxPrimitiveVertShader", defaultVertexBoxDrawShaderSource, ShaderType::Vertex);        // TODO: make vertex shader for windows surface configurable/scriptable
	_fragmentShader = shaderManager.loadShaderFromMemory("BoxPrimitiveFragShader", defaultFragmentBoxDrawShaderSource, ShaderType::Fragment); // TODO: make fragment shader for windows surface configurable/scriptable

	//---
	// box filled pipeline
	PipelineBuilder boxFilledPipelineBuilder;
	PipelineDefinition boxFilledDefinition = boxFilledPipelineBuilder
												 .setResourceLayout(ResourceLayoutBuilder()
																		// topology
																		.setTopology(PrimitiveTopology::TriangleList)

																		// vertex shader stage
																		.setVertexType<BoxVertex>()
																		.addStorageBuffer<uint8_t>(ShaderStage::Vertex, 0)      // palette buffer
																		.addPushConstant<BoxPushConstants>(ShaderStage::Vertex) // push constant for screen width and height

																		.build())
												 .setVertexShader(*_vertexShader)
												 .setFragmentShader(*_fragmentShader)
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
																		 .addStorageBuffer<uint8_t>(ShaderStage::Vertex, 0)      // palette buffer
																		 .addPushConstant<BoxPushConstants>(ShaderStage::Vertex) // push constant for screen width and height

																		 .build())
												  .setVertexShader(*_vertexShader)
												  .setFragmentShader(*_fragmentShader)
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
