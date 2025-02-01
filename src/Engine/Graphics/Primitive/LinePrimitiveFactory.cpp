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

#include "../Pipeline.h"
#include "../PipelineDefinition.h"
#include "../PipelineManager.h"
#include "../ShaderManager.h"
#include "../Shader.h"

#include "../../EngineContext.h"
#include "../../Resource/ResourceSystem.h"

namespace OpenXcom
{

// Vertex shader used to render the game surface to the platform window screen
const char* defaultVertexLineDrawShaderSource = R"(
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
		vec2 ndc = (((vec2(snappedPosition) + vec2(0.5, 0.5)) / vec2(pushConstants.screenWidth, pushConstants.screenHeight)) * 2.0 - 1.0) * 1.001;

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
const char* defaultFragmentLineDrawShaderSource = R"(
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


LinePrimitiveFactory::LinePrimitiveFactory(EngineContext& context, RenderTarget& surface)
	: _context(context), _surface(surface)
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	ShaderManager& shaderManager = resourceSystem.getShaderManager();
	PipelineManager& pipelineManager = resourceSystem.getPipelineManager();

	// load the shaders
	_vertexShader = shaderManager.loadShaderFromMemory("LinePrimitiveVertShader", defaultVertexLineDrawShaderSource, ShaderType::Vertex); // TODO: make vertex shader for windows surface configurable/scriptable
	_fragmentShader = shaderManager.loadShaderFromMemory("LinePrimitiveFragShader", defaultFragmentLineDrawShaderSource, ShaderType::Fragment); // TODO: make fragment shader for windows surface configurable/scriptable

	//---
	// line list pipeline
	PipelineBuilder lineListPipelineBuilder;
	PipelineDefinition lineListDefinition = lineListPipelineBuilder
										.setResourceLayout(ResourceLayoutBuilder()
															   // topology
															   .setTopology(PrimitiveTopology::LineList)

															   // vertex shader stage
															   .setVertexType<LineVertex>()
															   .addStorageBuffer<uint8_t>(ShaderStage::Vertex, 0)   // palette buffer
															   .addPushConstant<LinePushConstants>(ShaderStage::Vertex) // push constant for screen width and height

															   .build())
										.setVertexShader(*_vertexShader)
										.setFragmentShader(*_fragmentShader)
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
															   .addStorageBuffer<uint8_t>(ShaderStage::Vertex, 0)       // palette buffer
															   .addPushConstant<LinePushConstants>(ShaderStage::Vertex) // push constant for screen width and height

															   .build())
										.setVertexShader(*_vertexShader)
										.setFragmentShader(*_fragmentShader)
										.setRenderTarget(_surface)
										.build();

	_lineStripPipeline = pipelineManager.createPipeline(lineStripDefinition);
}

LinePrimitiveFactory::~LinePrimitiveFactory()
{
}

std::unique_ptr<LineListPrimitive> LinePrimitiveFactory::createLineListPrimitive(const LineVertex* lines, size_t count, int color, const ResourceHandle<Palette>& palette)
{
	return std::make_unique<LineListPrimitive>(_context, *_lineListPipeline, _surface, lines, count, color, palette);
}

std::unique_ptr<LineStripPrimitive> LinePrimitiveFactory::createLineStripPrimitive(const LineVertex* lines, size_t count, int color, const ResourceHandle<Palette>& palette)
{
	return std::make_unique<LineStripPrimitive>(_context, *_lineStripPipeline, _surface, lines, count, color, palette);
}

} // namespace OpenXcom
