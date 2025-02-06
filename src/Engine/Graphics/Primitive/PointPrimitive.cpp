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
#include "PointPrimitive.h"

#include "../Buffer/Buffer.h"
#include "../Buffer/BufferManager.h"
#include "../GraphicsCommand.h"
#include "../GraphicsSurface.h"
#include "../Palette/Palette.h"
#include "../Palette/PaletteManager.h"
#include "../Pipeline.h"
#include "../PipelineBinding.h"
#include "../PipelineDefinition.h"
#include "../Shader.h"

#include "../../EngineContext.h"
#include "../../Resource/ResourceSystem.h"

#include "../../Utility/RTTR.h"

SIMPLERTTR
{
	SimpleRTTR::registration().type<OpenXcom::PointVertex>()
		.property(&OpenXcom::PointVertex::pos, "pos");

	SimpleRTTR::registration().type<OpenXcom::PointColorVertex>()
		.property(&OpenXcom::PointColorVertex::pos, "pos")
		.property(&OpenXcom::PointColorVertex::color, "color");

	SimpleRTTR::registration().type<OpenXcom::PointPushConstants>()
		.property(&OpenXcom::PointPushConstants::color, "color");
}

namespace OpenXcom
{

PointListPrimitive::PointListPrimitive(EngineContext& context, Pipeline& pipeline, RenderTarget& surface, const PointVertex* points, size_t count, int color, const ResourceHandle<Palette>& paletteHandle)
{
	ResourceSystem& resourceSystem = context.getResourceSystem();
	BufferManager& bufferManager = resourceSystem.getBufferManager();
	PaletteManager& paletteManager = resourceSystem.getPaletteManager();

	Palette& palette = paletteManager.get(paletteHandle);

	_pipelineBinding = pipeline.createBinding();

	_vertexBuffer = bufferManager.createDeviceBuffer<PointVertex>(points, count, BufferUsage::Vertex);
	_pipelineBinding->setVertexBuffer(*_vertexBuffer);

	_pipelineBinding->setUniformBuffer(ShaderStage::Vertex, 0, surface.getDeviceImageData());	// bind the surface extents
	_pipelineBinding->setUniformBuffer(ShaderStage::Vertex, 1, palette.getDeviceBuffer());

	PointPushConstants pushConstants{color};
	_pipelineBinding->setPushConstant<PointPushConstants>(ShaderStage::Vertex, pushConstants);
}

PointListPrimitive::~PointListPrimitive()
{
}

void PointListPrimitive::draw(GraphicsCommand& command)
{
	_pipelineBinding->commit(command);
}


PointColorListPrimitive::PointColorListPrimitive(EngineContext& context, Pipeline& pipeline, RenderTarget& surface, const PointColorVertex* points, size_t count, const ResourceHandle<Palette>& paletteHandle)
{
	ResourceSystem& resourceSystem = context.getResourceSystem();
	BufferManager& bufferManager = resourceSystem.getBufferManager();
	PaletteManager& paletteManager = resourceSystem.getPaletteManager();

	Palette& palette = paletteManager.get(paletteHandle);

	_pipelineBinding = pipeline.createBinding();

	_vertexBuffer = bufferManager.createDeviceBuffer<PointColorVertex>(points, count, BufferUsage::Vertex);
	_pipelineBinding->setVertexBuffer(*_vertexBuffer);

	_pipelineBinding->setUniformBuffer(ShaderStage::Vertex, 0, surface.getDeviceImageData()); // bind the surface extents
	_pipelineBinding->setUniformBuffer(ShaderStage::Vertex, 1, palette.getDeviceBuffer());
}

PointColorListPrimitive::~PointColorListPrimitive()
{
}

void PointColorListPrimitive::draw(GraphicsCommand& command)
{
	_pipelineBinding->commit(command);
}

} // namespace OpenXcom
