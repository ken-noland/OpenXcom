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
#include "../Engine/Engine.h"
#include "../Engine/Platform/WindowSystem.h"
#include "../Engine/Platform/Window.h"

#include "../Engine/Graphics/GraphicsSystem.h"
#include "../Engine/Graphics/GraphicsSurface.h"
#include "../Engine/Graphics/GraphicsCommand.h"
#include "../Engine/Graphics/Buffer.h"
#include "../Engine/Graphics/BufferManager.h"
#include "../Engine/Graphics/Pipeline.h"
#include "../Engine/Graphics/PipelineBinding.h"
#include "../Engine/Graphics/PipelineDefinition.h"
#include "../Engine/Graphics/PipelineManager.h"
#include "../Engine/Graphics/ShaderManager.h"
#include "../Engine/Graphics/Shader.h"

#include "../Engine/Resource/ResourceSystem.h"
#include "../Engine/Resource/Image/ImageManager.h"
#include "../Engine/Resource/Image/Image.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <limits>

#include <SimpleRTTR.h>

namespace OpenXcom
{


// Vertex shader used to render the game surface to the platform window screen
const char* vertexShaderSource = R"(
	#version 450

	layout(location = 0) in vec2 inPosition;
	layout(location = 1) in vec2 inTexCoord;
	layout(location = 0) out vec2 fragTexCoord;

	layout(push_constant) uniform PushConstants {
		mat4 transform;
	} pushConstants;

	void main() {
		gl_Position = pushConstants.transform * vec4(inPosition, 0.0, 1.0);
		fragTexCoord = inTexCoord;
	}
)";

// Fragment shader used to render the game surface to the platform window screen
const char* fragmentShaderSource = R"(
	#version 450
	layout(location = 0) in vec2 fragTexCoord;     // Input texture coordinates from vertex shader

	layout(binding = 0) uniform sampler2D uTexture; // Texture sampler, bound to descriptor set

	layout(location = 0) out vec4 outColor;        // Output color

	void main() {
		vec4 texColor = texture(uTexture, fragTexCoord);
		outColor = texColor;
	}
)";


struct GameScreenVertex
{
	glm::vec2 pos;
	glm::vec2 texCoord;
};

GameScreenVertex vertices[] = {
	{{-1.0f, -1.0f}, {0.0f, 0.0f}}, // Bottom-left
	{{1.0f, -1.0f}, {1.0f, 0.0f}},  // Bottom-right
	{{1.0f, 1.0f}, {1.0f, 1.0f}},   // Top-right
	{{-1.0f, 1.0f}, {0.0f, 1.0f}}   // Top-left
};

// Indices for two triangles forming a rectangle
uint16_t indices[] = {0, 1, 2, 2, 3, 0};


// Run time type information
SIMPLERTTR
{
	SimpleRTTR::registration().type<glm::vec2>()
		.meta("format", 0) /* vk::Format::eR32G32Sfloat */
		.property(&glm::vec2::x, "x")
		.property(&glm::vec2::y, "y");

	SimpleRTTR::registration().type<glm::vec3>()
		.meta("format", 1) /* vk::Format::eR32G32B32Sfloat */
		.property(&glm::vec3::x, "x")
		.property(&glm::vec3::y, "y")
		.property(&glm::vec3::z, "z");

	SimpleRTTR::registration().type<OpenXcom::GameScreenVertex>()
		.property(&OpenXcom::GameScreenVertex::pos, "pos")
		.property(&OpenXcom::GameScreenVertex::texCoord, "texCoord");

	SimpleRTTR::registration().type<glm::mat4>()
		.meta("format", 1); /* vk::Format::eR32G32B32Sfloat */
}

GameWindow::GameWindow(const std::string& title)
{
	// create the window
	Engine& engine = getEngine();
	_window = engine.getPlatformWindowSystem().createWindow("Title", 1024, 768).lock();

	// load the shaders
	_vertexShader = engine.getResourceSystem().getShaderManager().loadShaderFromMemory("WindowSurfaceVertShader", vertexShaderSource, ShaderType::Vertex);
	_fragmentShader = engine.getResourceSystem().getShaderManager().loadShaderFromMemory("WindowSurfaceFragShader", fragmentShaderSource, ShaderType::Fragment);

	// create a graphics surface for the window
	GraphicsSystem& graphicsSystem = engine.getGraphicsSystem();
	_graphicsSurface = graphicsSystem.createSurface(_window->getHandle());


	// create a render target for the game surface
	_gameSurface = graphicsSystem.createRenderTarget(320, 200, ImageFormat::RGBA8);

	PipelineBuilder pipelineBuilder;

	PipelineDefinition pipeline = pipelineBuilder
		.setVertexShader(*_vertexShader)
		.setResourceLayout(ResourceLayoutBuilder()
			// vertex shader stage
			.setVertexType<GameScreenVertex>()
			.addPushConstant<glm::mat4>(ShaderStage::Vertex)

			// fragment shader stage
			.addCombinedImageSampler(0, ShaderStage::Fragment)
			.addTexture(0, ShaderStage::Fragment)
			.build())
		.setFragmentShader(*_fragmentShader)
		.setSurface(*_graphicsSurface)
		.build();

	_windowPipeline = engine.getResourceSystem().getPipelineManager().createPipeline(pipeline);
	_windowPipelineBinding = _windowPipeline->createBinding();

	_vertexBuffer = engine.getResourceSystem().getBufferManager().createDeviceBuffer<GameScreenVertex>(vertices, 4, BufferUsage::Vertex);
	_indexBuffer = engine.getResourceSystem().getBufferManager().createDeviceBuffer<uint16_t>(indices, 6, BufferUsage::Index);

	//_windowPipelineBinding->setVertexBuffer(*_vertexBuffer);
	//_windowPipelineBinding->setIndexBuffer(*_indexBuffer);

	_window->show();
}

GameWindow::~GameWindow()
{
	Engine& engine = getEngine();

	// unload the shaders
	_vertexShader.reset();
	_fragmentShader.reset();

	// destroy the surface
	_graphicsSurface.reset();

	// destroy the window
	engine.getPlatformWindowSystem().destroyWindow(_window);
}

void GameWindow::update()
{
	//Engine& engine = getEngine();

	// update the window
	_window->update();

    GraphicsCommand& command = _graphicsSurface->beginCommandPass();

	// Render the game surface
	{
		command.beginRenderPass(*_gameSurface);
		// Perform game-specific rendering
		command.endRenderPass();
	}

	// Render the window surface
	{
		command.beginRenderPass(*_graphicsSurface);

		// most of this is for testing so we can validate the low level rendering
		////////////////////////////////////////////
		command.bindPipeline(*_windowPipeline);



//		command.bindRenderObject();

//		command.drawIndexed();
		////////////////////////////////////////////


		command.endRenderPass();
	}

	_graphicsSurface->endCommandPass(command);
}

} // namespace OpenXcom
