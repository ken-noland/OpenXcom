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
		int paletteColor;
	} pushConstants;

	// Uniform buffer for screen dimensions (set 0, binding 0)
	layout(set = 0, binding = 0, std140) uniform ScreenInfo {
		int screenWidth;
		int screenHeight;
	} screenInfo;

	// Buffer for palette colors (set 0, binding 1)
	layout(set = 0, binding = 1, std430) buffer Palette {
		uint colors[];
	};

	void main() {
		// Convert screen coordinates to normalized device coordinates (NDC)
		ivec2 snappedPosition = inPosition; // Already in integer format
		vec2 ndc = (((vec2(snappedPosition) + vec2(0.5, 0.5)) / 
					vec2(screenInfo.screenWidth, screenInfo.screenHeight)) * 2.0 - 1.0);

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

	layout(location = 0) out vec4 fragColor;     // Output color

	// Uniform buffer for screen dimensions (set 0, binding 0)
	layout(set = 0, binding = 0, std140) uniform ScreenInfo {
		int screenWidth;
		int screenHeight;
	} screenInfo;

	// Buffer for palette colors (set 0, binding 1)
	layout(set = 0, binding = 1, std430) buffer Palette {
		uint colors[];
	};

	void main() {
		// Convert screen coordinates to normalized device coordinates (NDC)
		ivec2 snappedPosition = inPosition; // Already in integer format
		vec2 ndc = (((vec2(snappedPosition) + vec2(0.5, 0.5)) / 
					vec2(screenInfo.screenWidth, screenInfo.screenHeight)) * 2.0 - 1.0);

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



const char* defaultVec2UVVertexShaderSource = R"(
	#version 450

	// Input vertex attributes: position and UV, both in integer (pixel) coordinates.
	layout(location = 0) in ivec2 inPosition;
	layout(location = 1) in ivec2 inUV;

	// Output to the fragment shader: normalized texture coordinates.
	layout(location = 0) out vec2 fragUV;

	// Uniform buffer for screen dimensions (set 0, binding 0)
	layout(set = 0, binding = 0, std140) uniform ScreenInfo {
		int screenWidth;
		int screenHeight;
	} screenInfo;

	// Uniform buffer for the source image (texture) size. (set 0, binding 1)
	layout(set = 0, binding = 1, std140) uniform SourceSize {
		ivec2 sourceSize;
	};

	void main() {
		// Convert screen coordinates to normalized device coordinates (NDC)
		ivec2 snappedPosition = inPosition; // Already in integer format
		vec2 ndc = (((vec2(snappedPosition) + vec2(0.5, 0.5)) / 
					vec2(screenInfo.screenWidth, screenInfo.screenHeight)) * 2.0 - 1.0);

		// Note: Depending on your coordinate system you might need to flip the Y axis.
		gl_Position = vec4(ndc, 0.0, 1.0);

		// Convert the source image coordinates (UV) to normalized coordinates for texture sampling.
		fragUV = (vec2(inUV) + vec2(0.5)) / vec2(sourceSize);
	}
)";

const char* defaultUVFragmentShaderSource = R"(
	#version 450

	// Input UV coordinates from the vertex shader.
	layout(location = 0) in vec2 fragUV;

	// Output final color.
	layout(location = 0) out vec4 fragColor;

	// R8 texture (the image) bound to set 0, binding 0.
	layout(set = 0, binding = 2) uniform sampler2D uImage;

	// Palette
	layout(set = 0, binding = 3, std430) buffer PaletteBuffer {
		uint palette[];
	};


	void main() {
		// Sample the indexed image. For an R8 texture, the red channel will contain the index
		// in normalized form (i.e. in the range [0.0, 1.0]).
		float indexNormalized = texture(uImage, fragUV).r;
    
		// Convert the normalized value to an integer index.
		// For an 8-bit channel, multiply by 255 and round.
		uint index = uint(round(indexNormalized * 255.0));

		// Look up the color in the palette.
		uint packedColor = palette[index];

		// Unpack the color assuming it is stored as 0xRRGGBBAA.
		fragColor = vec4(
			float((packedColor >> 24) & 0xFF) / 255.0,  // Red
			float((packedColor >> 16) & 0xFF) / 255.0,  // Green
			float((packedColor >> 8)  & 0xFF) / 255.0,  // Blue
			float((packedColor >> 0)  & 0xFF) / 255.0   // Alpha
		);
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
	_defaultVec2UvShader = shaderManager.loadShaderFromMemory("defaultVec2UVVertexShader", defaultVec2UVVertexShaderSource, ShaderType::Vertex);

	_defaultFragmentShader = shaderManager.loadShaderFromMemory("defaultFragmentShaderSource", defaultFragmentShaderSource, ShaderType::Fragment);
	_defaultUVFragmentShader = shaderManager.loadShaderFromMemory("defaultUVFragmentShaderSource", defaultUVFragmentShaderSource, ShaderType::Fragment);
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

Shader& ShaderCollection::getDefaultVec2UVVertexShader()
{
	return *_defaultVec2UvShader;
}

Shader& ShaderCollection::getDefaultFragmentShader()
{
	return *_defaultFragmentShader;
}

Shader& ShaderCollection::getDefaultUVFragmentShader()
{
	return *_defaultUVFragmentShader;
}

} // namespace OpenXcom
