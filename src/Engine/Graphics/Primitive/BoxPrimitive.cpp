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
#include "BoxPrimitive.h"
#include "../GraphicsCommand.h"
#include "../GraphicsSurface.h"
#include "../Buffer.h"
#include "../BufferManager.h"
#include "../PipelineBinding.h"
#include "../PipelineDefinition.h"
#include "../Pipeline.h"
#include "../Shader.h"
#include "../Palette/Palette.h"
#include "../Palette/PaletteManager.h"

#include "../../Resource/ResourceSystem.h"
#include "../../EngineContext.h"

#include "../../Utility/RTTR.h"

SIMPLERTTR
{
	SimpleRTTR::registration().type<OpenXcom::BoxVertex>()
		.property(&OpenXcom::BoxVertex::pos, "pos");

	SimpleRTTR::registration().type<OpenXcom::BoxPushConstants>()
		.property(&OpenXcom::BoxPushConstants::color, "color");
}

namespace OpenXcom
{

BoxFilledPrimitive::BoxFilledPrimitive(EngineContext& context, Pipeline& pipeline, RenderTarget& surface,
									   glm::ivec2 position, glm::ivec2 extent, int color, const ResourceHandle<Palette>& paletteHandle)
{
	ResourceSystem& resourceSystem = context.getResourceSystem();
	BufferManager& bufferManager = resourceSystem.getBufferManager();
	PaletteManager& paletteManager = resourceSystem.getPaletteManager();

	Palette& palette = paletteManager.get(paletteHandle);

	_pipelineBinding = pipeline.createBinding();

	glm::ivec2 extent1 = extent + glm::ivec2(1, 1); // +1 to include the right and bottom edges

	// Define two triangles that form a quad
	BoxVertex vertices[6] = {
		{position},                           // Top-left
		{position + glm::ivec2(extent1.x, 0)}, // Top-right
		{position + glm::ivec2(0, extent1.y)}, // Bottom-left

		{position + glm::ivec2(extent1.x, 0)}, // Top-right
		{position + extent1},                  // Bottom-right
		{position + glm::ivec2(0, extent1.y)}  // Bottom-left
	};

	_vertexBuffer = bufferManager.createDeviceBuffer<BoxVertex>(vertices, 6, BufferUsage::Vertex);
	_pipelineBinding->setVertexBuffer(*_vertexBuffer);

	_pipelineBinding->setUniformBuffer(ShaderStage::Vertex, 0, surface.getDeviceImageData()); // bind the surface extents
	_pipelineBinding->setUniformBuffer(ShaderStage::Vertex, 1, palette.getDeviceBuffer());

	BoxPushConstants pushConstants{color};
	_pipelineBinding->setPushConstant(ShaderStage::Vertex, pushConstants);
}

BoxFilledPrimitive::~BoxFilledPrimitive()
{
}

void BoxFilledPrimitive::draw(GraphicsCommand& command)
{
	_pipelineBinding->commit(command);
}

BoxOutlinePrimitive::BoxOutlinePrimitive(EngineContext& context, Pipeline& pipeline, RenderTarget& surface,
										 glm::ivec2 position, glm::ivec2 extent, int color, const ResourceHandle<Palette>& paletteHandle)
{
	ResourceSystem& resourceSystem = context.getResourceSystem();
	BufferManager& bufferManager = resourceSystem.getBufferManager();
	PaletteManager& paletteManager = resourceSystem.getPaletteManager();

	Palette& palette = paletteManager.get(paletteHandle);

	_pipelineBinding = pipeline.createBinding();

	// Define the 4 corner points forming a closed loop (line strip)
	BoxVertex vertices[5] = {
		{position},                           // Top-left
		{position + glm::ivec2(extent.x, 0)}, // Top-right
		{position + extent},                  // Bottom-right **+1**
		{position + glm::ivec2(0, extent.y)}, // Bottom-left
		{position}                            // Closing the loop
	};

	_vertexBuffer = bufferManager.createDeviceBuffer<BoxVertex>(vertices, 5, BufferUsage::Vertex);
	_pipelineBinding->setVertexBuffer(*_vertexBuffer);

	_pipelineBinding->setUniformBuffer(ShaderStage::Vertex, 0, surface.getDeviceImageData()); // bind the surface extents
	_pipelineBinding->setUniformBuffer(ShaderStage::Vertex, 1, palette.getDeviceBuffer());

	BoxPushConstants pushConstants{color};
	_pipelineBinding->setPushConstant(ShaderStage::Vertex, pushConstants);
}

BoxOutlinePrimitive::~BoxOutlinePrimitive()
{
}

void BoxOutlinePrimitive::draw(GraphicsCommand& command)
{
	_pipelineBinding->commit(command);
}


} // namespace OpenXcom
