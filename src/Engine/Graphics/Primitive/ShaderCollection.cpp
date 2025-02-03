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
#include "ShaderCollection.h"

#include "../ShaderManager.h"
#include "../Shader.h"

#include "../../EngineContext.h"
#include "../../Resource/ResourceSystem.h"

namespace OpenXcom
{


const char* defaultVec2VertexShaderSource = R"(
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
		vec2 ndc = (((vec2(snappedPosition) + vec2(0.5, 0.5)) / vec2(pushConstants.screenWidth, pushConstants.screenHeight)) * 2.0 - 1.0);

		gl_Position = vec4(ndc, 0.0, 1.0);

		uint packedColor = colors[pushConstants.paletteColor];

		fragColor = vec4(
			float((packedColor >> 24) & 0xFF) / 255.0,  // Red
			float((packedColor >> 16) & 0xFF) / 255.0,  // Green
			float((packedColor >> 8)  & 0xFF) / 255.0,  // Blue
			float((packedColor >> 0)  & 0xFF) / 255.0   // Alpha
		);
	}
)";

const char* defaultVec2ColorVertexShaderSource = R"(
	#version 450

	layout(location = 0) in ivec2 inPosition;  // Input as signed integers
	layout(location = 1) in uint inColor;      // Input color

	layout(location = 0) out vec4 fragColor;  // Output color

	layout(push_constant) uniform PushConstants {
		int screenWidth;    //TODO: Move to uniform buffer
		int screenHeight;   //TODO: Move to uniform buffer
	} pushConstants;

	layout(set = 0, binding = 0, std430) buffer Palette {
		uint colors[];     // Dynamically sized palette
	};

	void main() {
		// Convert screen coordinates to normalized device coordinates (NDC)
		ivec2 snappedPosition = inPosition; // Already in integer format
		vec2 ndc = (((vec2(snappedPosition) + vec2(0.5, 0.5)) / vec2(pushConstants.screenWidth, pushConstants.screenHeight)) * 2.0 - 1.0);

		gl_Position = vec4(ndc, 0.0, 1.0);

		uint packedColor = colors[inColor];

		fragColor = vec4(
			float((packedColor >> 24) & 0xFF) / 255.0,  // Red
			float((packedColor >> 16) & 0xFF) / 255.0,  // Green
			float((packedColor >> 8)  & 0xFF) / 255.0,  // Blue
			float((packedColor >> 0)  & 0xFF) / 255.0   // Alpha
		);
	}
)";


// Fragment shader used to render the game surface to the platform window screen
const char* defaultFragmentShaderSource = R"(
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

ShaderCollection::ShaderCollection(EngineContext& context)
	: _context(context)
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	ShaderManager& shaderManager = resourceSystem.getShaderManager();

	// load the shaders
	_defaultVec2Shader = shaderManager.loadShaderFromMemory("defaultVec2VertexShader", defaultVec2VertexShaderSource, ShaderType::Vertex);
	_defaultVec2ColorShader = shaderManager.loadShaderFromMemory("defaultVec2ColorVertexShader", defaultVec2ColorVertexShaderSource, ShaderType::Vertex);

	_defaultFragmentShader = shaderManager.loadShaderFromMemory("LinePrimitiveFragShader", defaultFragmentShaderSource, ShaderType::Fragment);
}

ShaderCollection::~ShaderCollection()
{
}

Shader& ShaderCollection::getDefaultVec2VertexShader()
{
	return *_defaultVec2Shader;
}

Shader& ShaderCollection::getDefaultVec2ColorVertexShader()
{
	return *_defaultVec2ColorShader;
}

Shader& ShaderCollection::getDefaultFragmentShader()
{
	return *_defaultFragmentShader;
}



} // namespace OpenXcom
