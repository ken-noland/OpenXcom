#include "Inspector.h"
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
#include "Inspector.h"

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../GameContext.h"
#include "../../Engine/EngineContext.h"
#include "../../Engine/Logger.h"

#include "../../Engine/Platform/PlatformWindow.h"
#include "../../Engine/Platform/PlatformWindowSystem.h"

#include "../../Engine/Graphics/GraphicsSystem.h"
#include "../../Engine/Graphics/GraphicsSurface.h"
#include "../../Engine/Graphics/GraphicsCommand.h"

#include "../../Engine/Graphics/Shader.h"
#include "../../Engine/Graphics/ShaderManager.h"

#include "../../Engine/Graphics/Pipeline.h"
#include "../../Engine/Graphics/PipelineBinding.h"
#include "../../Engine/Graphics/PipelineManager.h"
#include "../../Engine/Graphics/PipelineDefinition.h"

#include "../../Engine/Graphics/Image/Image.h"
#include "../../Engine/Graphics/Image/ImageManager.h"

#include "../../Engine/Graphics/Buffer/Buffer.h"
#include "../../Engine/Graphics/Buffer/BufferManager.h"

#include "../../Engine/Resource/ResourceSystem.h"

namespace OpenXcom
{

const char* imguiVertexShaderSource = R"(
#version 450

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in uint inColor;

// Use a push constant for the projection matrix.
layout (push_constant) uniform PushConstants {
    mat4 proj;
} pushConstants;

layout (location = 0) out vec2 fragUV;
layout (location = 1) out vec4 fragColor;

void main()
{
    fragUV = inUV;
    fragColor = unpackUnorm4x8(inColor);
    gl_Position = pushConstants.proj * vec4(inPos, 0.0, 1.0);
}
)";

const char* imguiFragmentShaderSource = R"(
#version 450

layout (binding = 0) uniform sampler2D fontTexture;

layout (location = 0) in vec2 fragUV;
layout (location = 1) in vec4 fragColor;

layout (location = 0) out vec4 outColor;

void main()
{
    // Multiply the incoming color with the sampled texture color.
    outColor = fragColor * texture(fontTexture, fragUV);
}
)";

struct OpenXcomImDrawVert
{
	glm::vec2 pos;
	glm::vec2 uv;
	uint32_t col;
};

SIMPLERTTR
{
	SimpleRTTR::registration().type<OpenXcomImDrawVert>()
		.property(&OpenXcomImDrawVert::pos, "pos")
		.property(&OpenXcomImDrawVert::uv, "uv")
		.property(&OpenXcomImDrawVert::col, "col");
}

Inspector::Inspector(GameContext& game)
	: _game(game)
{
	EngineContext& engine = _game.getEngineContext();
	PlatformWindowSystem& windowSystem = engine.getPlatformWindowSystem();
	GraphicsSystem& graphicsSystem = engine.getGraphicsSystem();

	ImageManager& imageManager = engine.getResourceSystem().getImageManager();
	ShaderManager& shaderManager = engine.getResourceSystem().getShaderManager();
	PipelineManager& pipelineManager = engine.getResourceSystem().getPipelineManager();
	BufferManager& bufferManager = engine.getResourceSystem().getBufferManager();

	// create the window
	_window = engine.getPlatformWindowSystem().create("Inspector", 1024, 768); // TODO: use game options to set the window parameters

	// create a graphics surface for the window
	_windowSurface = graphicsSystem.createWindowedSurface(*_window);

	// Create ImGui context and set style.
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	// (Optionally) Add fonts and build the atlas.
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();

	// Retrieve font texture data and create a GPU texture.
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	size_t size = width * height * bytesPerPixel(ImageFormat::R8G8B8A8);

	// first we have to create a host image from the font texture data
	OwningHandle<HostImage> hostFontImage = imageManager.createHostImage("ImGuiFont", glm::ivec2(width, height), ImageFormat::R8G8B8A8);
	void* hostPixels = hostFontImage->map();
	memcpy(hostPixels, pixels, size);
	hostFontImage->unmap();

	// then we create a device image from the host image to finalize the font texture
	_font = imageManager.createDeviceImage(*hostFontImage);
	hostFontImage.reset(); // cleanup the host image

	// load the shaders
	_vertexShader = engine.getResourceSystem().getShaderManager().loadShaderFromMemory("ImGuiVertShader", imguiVertexShaderSource, ShaderType::Vertex);       // TODO: make vertex shader for windows surface configurable/scriptable
	_fragmentShader = engine.getResourceSystem().getShaderManager().loadShaderFromMemory("ImGuiFragShader", imguiFragmentShaderSource, ShaderType::Fragment); // TODO: make fragment shader for windows surface configurable/scriptable

	// now we set up the shaders and pipeline for rendering ImGui
	PipelineBuilder pipelineBuilder;
	PipelineDefinition imguiPipelineDefinition = pipelineBuilder
													.setVertexShader(*_vertexShader) // shader handling ImDrawVert attributes
													.setFragmentShader(*_fragmentShader)
													.setResourceLayout(ResourceLayoutBuilder()
																			.setVertexType<OpenXcomImDrawVert>() // ImGui's vertex layout(with GLM RTTR bindings).
																			.setIndexType<unsigned short>()      // ImGui's index layout.
																			.addPushConstant<glm::mat4>(ShaderStage::Vertex) // For the orthographic projection.

																			.addCombinedImageSampler(ShaderStage::Fragment, 0) // The font atlas texture.
																			.addTexture(ShaderStage::Fragment, 0)              // The font atlas texture.
																			.build())
													.setRenderTarget(*_windowSurface)
													.build();

	// create the pipeline
	_pipeline = pipelineManager.createPipeline(imguiPipelineDefinition);
	_pipelineBinding = _pipeline->createBinding();

	constexpr size_t vertexBufferCount = 16384;
	constexpr size_t indexBufferCount = 4096;

	// create the vertex and index buffers
	_hostVertexBuffer = bufferManager.createHostBuffer(sizeof(OpenXcomImDrawVert), vertexBufferCount, BufferUsage::Vertex);
	_hostIndexBuffer = bufferManager.createHostBuffer(sizeof(unsigned short), indexBufferCount, BufferUsage::Index);
	_deviceVertexBuffer = bufferManager.createDeviceBuffer(sizeof(OpenXcomImDrawVert), vertexBufferCount, BufferUsage::Vertex);
	_deviceIndexBuffer = bufferManager.createDeviceBuffer(sizeof(unsigned short), indexBufferCount, BufferUsage::Index);

	_pipelineBinding->setTexture(ShaderStage::Fragment, 0, *_font);
	_pipelineBinding->setVertexBuffer(*_deviceVertexBuffer);
	_pipelineBinding->setIndexBuffer(*_deviceIndexBuffer);
}

Inspector::~Inspector()
{
	ImGui::DestroyContext();

	_font.reset();
	_windowSurface.reset();
	_window.reset();
}

void Inspector::show()
{
	_window->show();
}

void Inspector::hide()
{
	_window->hide();
}

void Inspector::update()
{
	if (_window && !_window->isMinimized())
	{
		glm::ivec2 windowExtent = _windowSurface->getExtent();
		glm::vec2 windowSize = glm::vec2(windowExtent.x, windowExtent.y);
		_projection = glm::ortho(0.0f, windowSize.x, windowSize.y, 0.0f,
                        -1.0f, 1.0f);

		//update ImGui
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(windowSize.x, windowSize.y);

		// Start a new ImGui frame.
		ImGui::NewFrame();

		// Build your UI.
		ImGui::Begin("Inspector");
		// ... add widgets ...
		ImGui::End();

		// Render the UI.
		ImGui::Render();

		ImDrawData* drawData = ImGui::GetDrawData();

		/////////////////////////////////////////
		// BEGIN TEMP

		// Coordinates assume a window size of 1024x768.
		OpenXcomImDrawVert vertices[4] = {
			// Position         UV coordinates   Color (RGBA)
			{ {100.0f, 100.0f}, {0.0f, 0.0f}, 0xFF0000FF }, // bottom-left (red)
			{ {300.0f, 100.0f}, {1.0f, 0.0f}, 0x00FF00FF }, // bottom-right (green)
			{ {300.0f, 300.0f}, {1.0f, 1.0f}, 0x0000FFFF }, // top-right (blue)
			{ {100.0f, 300.0f}, {0.0f, 1.0f}, 0xFFFFFFFF }  // top-left (white)
		};

		uint16_t indices[6] = {
			0, 1, 2, // first triangle
			0, 2, 3  // second triangle
		};

		_hostVertexBuffer->set(vertices, 4);
		_hostIndexBuffer->set(indices, 6);

		_deviceVertexBuffer->copy(*_hostVertexBuffer);
		_deviceIndexBuffer->copy(*_hostIndexBuffer);

		// END TEMP
		/////////////////////////////////////////

		_pipelineBinding->setPushConstant(ShaderStage::Vertex, _projection);

		// begin the command pass(the start of rendering)
		GraphicsCommand& command = _windowSurface->beginCommandPass();

		// render the ImGui stuff here!
		command.beginRenderPass(*_windowSurface);

		_pipelineBinding->commit(command);

		command.endRenderPass();

		// end the command pass(the end of rendering)
		_windowSurface->endCommandPass(command);
	}
}

} // namespace OpenXcom

