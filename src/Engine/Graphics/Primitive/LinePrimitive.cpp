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
#include "LinePrimitive.h"
#include "../GraphicsCommand.h"
#include "../GraphicsSurface.h"
#include "../Buffer/Buffer.h"
#include "../Buffer/BufferManager.h"
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
	SimpleRTTR::registration().type<OpenXcom::LineVertex>()
		.property(&OpenXcom::LineVertex::pos, "pos");

	SimpleRTTR::registration().type<OpenXcom::LinePushConstants>()
		.property(&OpenXcom::LinePushConstants::color, "color");
}


namespace OpenXcom
{

LineListPrimitive::LineListPrimitive(EngineContext& context, Pipeline& pipeline, RenderTarget& surface,
									 const LineVertex* lines, size_t count, int color, const ResourceHandle<Palette>& paletteHandle)
	: _context(context)
{
	_pipelineBinding = pipeline.createBinding();

	setLines(lines, count);
	setPalette(paletteHandle);
	setPaletteColorIndex(color);
	setSurface(surface);
}

LineListPrimitive::~LineListPrimitive()
{
}

void LineListPrimitive::setLines(const LineVertex* lines, size_t count)
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	BufferManager& bufferManager = resourceSystem.getBufferManager();

	_vertexBuffer = bufferManager.createDeviceBuffer<LineVertex>(lines, count, BufferUsage::Vertex);
	_pipelineBinding->setVertexBuffer(*_vertexBuffer);
}

void LineListPrimitive::setPaletteColorIndex(int color)
{
	LinePushConstants pushConstants{color};
	_pipelineBinding->setPushConstant(ShaderStage::Vertex, pushConstants);
}

void LineListPrimitive::setPalette(const ResourceHandle<Palette>& paletteHandle)
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	PaletteManager& paletteManager = resourceSystem.getPaletteManager();

	Palette& palette = paletteManager.get(paletteHandle);
	_pipelineBinding->setUniformBuffer(ShaderStage::Vertex, 1, palette.getDeviceBuffer()); // bind the palette
}

void LineListPrimitive::setSurface(RenderTarget& surface)
{
	_pipelineBinding->setUniformBuffer(ShaderStage::Vertex, 0, surface.getDeviceImageData()); // bind the surface extents
}


void LineListPrimitive::draw(GraphicsCommand& command)
{
	_pipelineBinding->commit(command);
}

LineStripPrimitive::LineStripPrimitive(EngineContext& context, Pipeline& pipeline, RenderTarget& surface,
									   const LineVertex* lines, size_t count, int color, const ResourceHandle<Palette>& paletteHandle)
	: _context(context)
{
	_pipelineBinding = pipeline.createBinding();

	setLines(lines, count);
	setPalette(paletteHandle);
	setPaletteColorIndex(color);
	setSurface(surface);
}

LineStripPrimitive::~LineStripPrimitive()
{
}

void LineStripPrimitive::setLines(const LineVertex* lines, size_t count)
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	BufferManager& bufferManager = resourceSystem.getBufferManager();

	_vertexBuffer = bufferManager.createDeviceBuffer<LineVertex>(lines, count, BufferUsage::Vertex);
	_pipelineBinding->setVertexBuffer(*_vertexBuffer);
}

void LineStripPrimitive::setPaletteColorIndex(int color)
{
	LinePushConstants pushConstants{color};
	_pipelineBinding->setPushConstant(ShaderStage::Vertex, pushConstants);
}

void LineStripPrimitive::setPalette(const ResourceHandle<Palette>& paletteHandle)
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	PaletteManager& paletteManager = resourceSystem.getPaletteManager();

	Palette& palette = paletteManager.get(paletteHandle);
	_pipelineBinding->setUniformBuffer(ShaderStage::Vertex, 1, palette.getDeviceBuffer()); // bind the palette
}

void LineStripPrimitive::setSurface(RenderTarget& surface)
{
	_pipelineBinding->setUniformBuffer(ShaderStage::Vertex, 0, surface.getDeviceImageData()); // bind the surface extents
}


void LineStripPrimitive::draw(GraphicsCommand& command)
{
	_pipelineBinding->commit(command);
}

} // namespace OpenXcom
