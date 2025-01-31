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
#include "GameWindow.h"
#include "../Engine/Graphics/Common/WindowSurface.h"
#include "../Engine/Graphics/Common/GameSurface.h"
#include "../Engine/Graphics/GraphicsCommand.h"

#include <functional>

/////////////////////////////////////////////
// TEMP
#include "../Engine/Engine.h"

#include "../Engine/Graphics/GraphicsSystem.h"
#include "../Engine/Graphics/GraphicsSurface.h"
#include "../Engine/Graphics/Buffer.h"
#include "../Engine/Graphics/BufferManager.h"
#include "../Engine/Graphics/Pipeline.h"
#include "../Engine/Graphics/PipelineBinding.h"
#include "../Engine/Graphics/PipelineDefinition.h"
#include "../Engine/Graphics/PipelineManager.h"
#include "../Engine/Graphics/Shader.h"
#include "../Engine/Graphics/ShaderManager.h"

#include "../Engine/Resource/ResourceSystem.h"
/////////////////////////////////////////////


namespace OpenXcom
{

	
/////////////////////////////////////////////
// TEMP

// Vertex shader used to render the game surface to the platform window screen
const char* vertexLineDrawShaderSource = R"(
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
		vec2 ndc = (vec2(snappedPosition) + 0.5) / vec2(pushConstants.screenWidth, pushConstants.screenHeight) * 2.0 - 1.0;

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
const char* fragmentLineDrawShaderSource = R"(
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

struct LineVertex
{
	glm::ivec2 pos;
};

struct Palette
{
	uint32_t colors[256];
};

struct PushConstants
{
	glm::ivec2 screenSize;
	int paletteColor;
};

SIMPLERTTR
{
	SimpleRTTR::registration().type<LineVertex>()
		.property(&OpenXcom::LineVertex::pos, "pos");

	SimpleRTTR::registration().type<Palette>();

	SimpleRTTR::registration().type<PushConstants>()
		.property(&OpenXcom::PushConstants::screenSize, "screenSize")
		.property(&OpenXcom::PushConstants::paletteColor, "paletteColor");
}

LineVertex lineVerticesTemp[] = {
	{{0, 0}},     // Bottom-left
	{{320, 200}}, // Bottom-right
};



/////////////////////////////////////////////


GameWindow::GameWindow(EngineContext& engine)
{
	Options& options = engine.getOptions();

	_gameSurface = std::make_unique<GameSurface>(engine);
	_windowSurface = std::make_unique<WindowSurface>(engine, *_gameSurface);

	_windowSurface->onRender() << [this](GraphicsCommand& command) {
		this->onWindowRender(command);
	};

	_gameSurface->onRender() << [this](GraphicsCommand& command) {
		this->onGameRender(command);
	};

	/////////////////////////////////////////////
	// TEMP

	ResourceSystem& resourceSystem = engine.getResourceSystem();
	ShaderManager& shaderManager = resourceSystem.getShaderManager();
	PipelineManager& pipelineManager = resourceSystem.getPipelineManager();
	BufferManager& bufferManager = resourceSystem.getBufferManager();
	PaletteManager& paletteManager = resourceSystem.getPaletteManager();

	// load the shaders
	_vertexShader = shaderManager.loadShaderFromMemory("WindowSurfaceVertShader", vertexLineDrawShaderSource, ShaderType::Vertex); // TODO: make vertex shader for windows surface configurable/scriptable
	_fragmentShader = shaderManager.loadShaderFromMemory("WindowSurfaceFragShader", fragmentLineDrawShaderSource, ShaderType::Fragment); // TODO: make fragment shader for windows surface configurable/scriptable

	PipelineBuilder pipelineBuilder;
	PipelineDefinition definition = pipelineBuilder
										.setResourceLayout(ResourceLayoutBuilder()
															   // topology
															   .setTopology(PrimitiveTopology::LineList)

															   // vertex shader stage
															   .setVertexType<LineVertex>()
															   .addStorageBuffer<Palette>(ShaderStage::Vertex, 0)
															   .addPushConstant<PushConstants>(ShaderStage::Vertex) // push constant for screen width and height

															   .build())
										.setVertexShader(*_vertexShader)
										.setFragmentShader(*_fragmentShader)
										.setRenderTarget(_gameSurface->getRenderTarget())
										.build();

	_pipeline = pipelineManager.createPipeline(definition);

	_pipelineBinding = _pipeline->createBinding();

	_vertexBuffer = bufferManager.createDeviceBuffer<LineVertex>(lineVerticesTemp, 2, BufferUsage::Vertex);
	_pipelineBinding->setVertexBuffer(*_vertexBuffer);

	Palette paletteData = {{0x00000000, 0xFFFFFFFF}};
	_palette = bufferManager.createDeviceBuffer(&paletteData, 1, BufferUsage::Storage);
	_pipelineBinding->setUniformBuffer(ShaderStage::Vertex, 0, *_palette);

	PushConstants pushConstants = {glm::ivec2(_gameSurface->getScreenSize()), 1};
	_pipelineBinding->setPushConstant(ShaderStage::Vertex, pushConstants);

	/////////////////////////////////////////////
}

GameWindow::~GameWindow()
{
}

void GameWindow::update()
{
	_windowSurface->update();
}

bool GameWindow::isRunning() const
{
	return _windowSurface->isRunning();
}

void GameWindow::onWindowRender(GraphicsCommand& command)
{

}

void GameWindow::onGameRender(GraphicsCommand& command)
{
	_pipelineBinding->commit(command);
}


} // namespace OpenXcom
