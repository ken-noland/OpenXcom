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
	layout(location = 1) in vec4 inColor;     // Input color

	layout(location = 0) out vec4 fragColor;  // Output color

	layout(push_constant) uniform PushConstants {
		int screenWidth;
		int screenHeight;
	} pushConstants;

	void main() {
		// Convert screen coordinates to normalized device coordinates (NDC)
		ivec2 snappedPosition = inPosition; // Already in integer format
		vec2 ndc = (vec2(snappedPosition) + 0.5) / vec2(pushConstants.screenWidth, pushConstants.screenHeight) * 2.0 - 1.0;
		ndc.y = -ndc.y; // Vulkan's NDC has Y flipped compared to screen-space

		gl_Position = vec4(ndc, 0.0, 1.0);
		fragColor = inColor;
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
	glm::vec4 color;	//technically, the color should be a push constant
};

SIMPLERTTR
{
	SimpleRTTR::registration().type<LineVertex>()
		.property(&OpenXcom::LineVertex::pos, "pos")
		.property(&OpenXcom::LineVertex::color, "texCoord");
}


/////////////////////////////////////////////


GameWindow::GameWindow(const std::string& title, Options& options)
{
	_gameSurface = std::make_unique<GameSurface>(options);
	_windowSurface = std::make_unique<WindowSurface>(title, options, *_gameSurface);

	_windowSurface->onRender() << [this](GraphicsCommand& command) {
		this->onWindowRender(command);
	};

	_gameSurface->onRender() << [this](GraphicsCommand& command) {
		this->onGameRender(command);
	};


	/////////////////////////////////////////////
	// TEMP

	Engine& engine = getEngine();
	ResourceSystem& resourceSystem = engine.getResourceSystem();
	ShaderManager& shaderManager = resourceSystem.getShaderManager();
	PipelineManager& pipelineManager = resourceSystem.getPipelineManager();

	// load the shaders
	_vertexShader = shaderManager.loadShaderFromMemory("WindowSurfaceVertShader", vertexLineDrawShaderSource, ShaderType::Vertex); // TODO: make vertex shader for windows surface configurable/scriptable
	_fragmentShader = shaderManager.loadShaderFromMemory("WindowSurfaceFragShader", fragmentLineDrawShaderSource, ShaderType::Fragment); // TODO: make fragment shader for windows surface configurable/scriptable


	PipelineBuilder pipelineBuilder;
	PipelineDefinition definition = pipelineBuilder
										.setResourceLayout(ResourceLayoutBuilder()
															   // vertex shader stage
															   .setVertexType<LineVertex>()
															   .addPushConstant<glm::ivec2>(ShaderStage::Vertex) // push constant for screen width and height

															   .build())
										.setVertexShader(*_vertexShader)
										.setFragmentShader(*_fragmentShader)
										.setRenderTarget(_gameSurface->getRenderTarget())
										.build();

	_pipeline = pipelineManager.createPipeline(definition);

	_pipelineBinding = _pipeline->createBinding();

	// even more temp temp stuff... this should be moved to a uniform buffer
	glm::ivec2 screenSize = _gameSurface->getScreenSize();

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
}


} // namespace OpenXcom
