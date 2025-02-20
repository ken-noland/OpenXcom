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
#include "WindowSurface.h"
#include "GameSurface.h"

#include "../GraphicsSystem.h"
#include "../GraphicsSurface.h"
#include "../GraphicsCommand.h"
#include "../Buffer/Buffer.h"
#include "../Buffer/BufferManager.h"
#include "../PipelineBinding.h"
#include "../PipelineDefinition.h"
#include "../PipelineManager.h"
#include "../Pipeline.h"
#include "../Shader.h"
#include "../ShaderManager.h"
#include "../Image/ImageManager.h"
#include "../Image/Image.h"
#include "../../Engine.h"
#include "../../EngineContext.h"
#include "../../Options.h"
#include "../../Resource/ResourceSystem.h"
#include "../../Platform/Window.h"

#include <simplerttr.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

struct WindowScreenVertex
{
	glm::vec2 pos;
	glm::vec2 texCoord;
};

WindowScreenVertex vertices[] = {
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
	SimpleRTTR::registration().type<OpenXcom::WindowScreenVertex>()
		.property(&OpenXcom::WindowScreenVertex::pos, "pos")
		.property(&OpenXcom::WindowScreenVertex::texCoord, "texCoord");
}


WindowSurface::WindowSurface(EngineContext& engine, GameSurface& gameSurface)
	: _engine(engine), _gameSurface(gameSurface), _isRunning(true)
{
	Options& options = engine.getOptions();

	if(options.get<&GraphicsOptions::_headless>() == false)
	{
		createWindowed(gameSurface);
	}
	else
	{
		createHeadless();
	}
}

WindowSurface::~WindowSurface()
{
}

void WindowSurface::createWindowed(GameSurface& gameSurface)
{
	// create the window
	_window = std::make_unique<PlatformWindow>(_engine.getTitle(), 1024, 768); // TODO: use game options to set the window parameters

	// create a graphics surface for the window
	GraphicsSystem& graphicsSystem = _engine.getGraphicsSystem();
	_windowSurface = graphicsSystem.createWindowedSurface(*_window);

	// load the shaders
	_vertexShader = _engine.getResourceSystem().getShaderManager().loadShaderFromMemory("WindowSurfaceVertShader", vertexShaderSource, ShaderType::Vertex);       // TODO: make vertex shader for windows surface configurable/scriptable
	_fragmentShader = _engine.getResourceSystem().getShaderManager().loadShaderFromMemory("WindowSurfaceFragShader", fragmentShaderSource, ShaderType::Fragment); // TODO: make fragment shader for windows surface configurable/scriptable

	PipelineBuilder pipelineBuilder;

	PipelineDefinition pipelineDefinition = pipelineBuilder
												.setVertexShader(*_vertexShader)
												.setResourceLayout(ResourceLayoutBuilder()
																	   // vertex shader stage
																	   .setVertexType<WindowScreenVertex>()
																	   .setIndexType<uint16_t>()
																	   .addPushConstant<glm::mat4>(ShaderStage::Vertex)

																	   // fragment shader stage
																	   .addCombinedImageSampler(ShaderStage::Fragment, 0)
																	   .addTexture(ShaderStage::Fragment, 0)
																	   .build())
												.setFragmentShader(*_fragmentShader)
												.setRenderTarget(*_windowSurface)
												.build();

	_pipeline = _engine.getResourceSystem().getPipelineManager().createPipeline(pipelineDefinition);
	_pipelineBinding = _pipeline->createBinding();

	_vertexBuffer = _engine.getResourceSystem().getBufferManager().createDeviceBuffer<WindowScreenVertex>(vertices, 4, BufferUsage::Vertex);
	_indexBuffer = _engine.getResourceSystem().getBufferManager().createDeviceBuffer<uint16_t>(indices, 6, BufferUsage::Index);

	_pipelineBinding->setVertexBuffer(*_vertexBuffer);
	_pipelineBinding->setIndexBuffer(*_indexBuffer);
	_pipelineBinding->setPushConstant(ShaderStage::Vertex, _projection);

	const DeviceImage& gameRenderTarget = gameSurface.getRenderTarget();
	_pipelineBinding->setTexture(ShaderStage::Fragment, 0, gameRenderTarget);

	// setup up the window projection
	updateProjection();

	// setup the callbacks
	_onClose = _window->onClose().add(std::bind(&WindowSurface::onClose, this));
	_onResize = _window->onResize().add(std::bind(&WindowSurface::onResize, this, std::placeholders::_1));

	_window->show();

	_isHeadless = false;
}

void WindowSurface::createHeadless()
{
	// create a graphics surface for the window
	GraphicsSystem& graphicsSystem = _engine.getGraphicsSystem();
	_windowSurface = graphicsSystem.createHeadlessSurface();

	_isHeadless = true;
}


void WindowSurface::update()
{
	if (!_isHeadless)
	{
		// update the window
		_window->update();
		
		if (!_isRunning)
		{
			_onClose.reset();
			_onResize.reset();

			if(_window)
			{
				_window.reset();
			}

			return;
		}
		
		//---
		// Windowed rendering

		if (!_window->isMinimized())
		{
			// begin the command pass(the start of rendering)
			GraphicsCommand& command = _windowSurface->beginCommandPass();

			// render the game surface
			_gameSurface.render(command);

			// render the game surface to the window surface
			command.beginRenderPass(*_windowSurface);
			_pipelineBinding->commit(command);

			// render delegates after the game surface
			_onRender.call(command);

			command.endRenderPass();

			// end the command pass(the end of rendering)
			_windowSurface->endCommandPass(command);
		}
	}
	else
	{
		//---
		// Headless rendering
		
		// begin the command pass(the start of rendering)
		GraphicsCommand& command = _windowSurface->beginCommandPass();

		// render the game surface
		_gameSurface.render(command);

		// end the command pass(the end of rendering)
		_windowSurface->endCommandPass(command);
	}
}

RenderTarget& WindowSurface::getRenderTarget()
{
	return *_windowSurface;
}

PlatformWindow& WindowSurface::getWindow()
{
	return *_window;
}

void WindowSurface::bindGameSurface(GameSurface& gameSurface)
{
	const DeviceImage& gameRenderTarget = gameSurface.getRenderTarget();
	_pipelineBinding->setTexture(ShaderStage::Fragment, 0, gameRenderTarget);
}


void WindowSurface::onResize(glm::ivec2 size)
{
	updateProjection();
}

void WindowSurface::onClose()
{
	if(_windowSurface)
	{
		_windowSurface.reset();
	}

	_engine.getEngine().exit();
	_isRunning = false;
}

void WindowSurface::updateProjection()
{
	float gameWidth = static_cast<float>(_gameSurface.getRenderTarget().getWidth());
	float gameHeight = static_cast<float>(_gameSurface.getRenderTarget().getHeight());
	float windowWidth = static_cast<float>(_windowSurface->getWidth());
	float windowHeight = static_cast<float>(_windowSurface->getHeight());

	float scaleX = 1.0f, scaleY = 1.0f;
	if (windowWidth / windowHeight > gameWidth / gameHeight)
	{
		scaleX = (gameWidth / gameHeight) / (windowWidth / windowHeight);
	}
	else
	{
		scaleY = (windowWidth / windowHeight) / (gameWidth / gameHeight);
	}

	_projection = glm::scale(glm::mat4(1.0f), glm::vec3(scaleX, scaleY, 1.0f));
	_pipelineBinding->setPushConstant(ShaderStage::Vertex, _projection);
}

} // namespace OpenXcom
