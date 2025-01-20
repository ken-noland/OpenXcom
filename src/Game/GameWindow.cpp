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
#include "../Engine/Resource/ResourceSystem.h"
#include "../Engine/Resource/Shader/ShaderManager.h"
#include "../Engine/Resource/Shader/Shader.h"
#include "../Engine/Resource/Pipeline/PipelineDefinition.h"
#include "../Engine/Resource/Pipeline/Pipeline.h"
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
	_renderTargetImage = engine.getResourceSystem().getImageManager().createRenderTarget(320, 200, ImageFormat::RGBA8);

	// TODO: Move all of this into GraphicsSystem(api agnostic) so we don't see this stuff clogging up the game code
	PipelineBuilder pipelineBuilder;

	PipelineDefinition pipeline = pipelineBuilder
		.setVertexShader(_vertexShader)
		.setResourceLayout(ResourceLayoutBuilder()
			// vertex shader stage
			.setVertexType<GameScreenVertex>()
			.addPushConstant<glm::mat4>(ShaderStage::Vertex)

			// fragment shader stage
			.addCombinedImageSampler(0, ShaderStage::Fragment)
			.addTexture(0, ShaderStage::Fragment)
			.build())
		.setFragmentShader(_fragmentShader)
		.setSurface(*_graphicsSurface)
		.build();

	//// define the pipeline
	//ResourceLayoutBuilder resourceLayout;

	//resourceLayout
	//	.addBinding(UBOBuilder()
	//		.setBinding(0)
	//		.setStageFlags(Stage::Vertex)
	//		.build())
	//	.addBinding(SamplerBuilder()
	//		.setBinding(1)
	//		.setStageFlags(Stage::Fragment)
	//		.build())
	//	.addBinding(TextureBuilder()
	//		.setBinding(2)
	//		.setStageFlags(Stage::Fragment)
	//		.build());

//	PipelineDescriptionBuilder pipelineDescription;

	//pipeline

	//	.setVertexShader(_vertexShader)
	//	.setFragmentShader(_fragmentShader)
	//	.setResourceLayout(resourceLayout.build())
	//	.setRenderTarget(_renderTargetImage)
	//	.build();


	_window->show();
}

GameWindow::~GameWindow()
{
	Engine& engine = getEngine();

	// unload the shaders
	ShaderManager& shaderManager = engine.getResourceSystem().getShaderManager();
	shaderManager.remove(_vertexShader);
	shaderManager.remove(_fragmentShader);

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

	// render the game surface to the window
	_graphicsSurface->draw();

	//// update the game surface
	//RenderTargetImage& renderTargetImage = *_renderTargetImage;
	//renderTargetImage.clear(Color(0, 0, 0, 255));

	//// render the game surface
	//{
	//	// bind the render target
	//	_graphicsSurface->bindRenderTarget(renderTargetImage);
	//	// bind the shaders
	//	ShaderManager& shaderManager = engine.getResourceSystem().getShaderManager();
	//	Shader& vertexShader = shaderManager.get(_vertexShader);
	//	Shader& fragmentShader = shaderManager.get(_fragmentShader);
	//	// bind the shaders
	//	_graphicsSurface->bindShader(vertexShader, fragmentShader);
	//	// bind the game surface
	//	_graphicsSurface->bindTexture(0, renderTargetImage);
	//	// bind the push constants
	//	_graphicsSurface->bindPushConstants(0, sizeof(glm::mat4), &_transform);
	//	// draw the game surface
	//	_graphicsSurface->draw();
	//	// unbind the game surface
	//	_graphicsSurface->unbindTexture(0);
	//	// unbind the shaders
	//	_graphicsSurface->unbindShader();
	//	// unbind the render target
	//	_graphicsSurface->unbindRenderTarget();
	//}
	//// present the game surface
	//_graphicsSurface->present();
}

} // namespace OpenXcom
