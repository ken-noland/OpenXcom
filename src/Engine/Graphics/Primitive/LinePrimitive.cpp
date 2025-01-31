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
#include "../Buffer.h"
#include "../BufferManager.h"
#include "../PipelineBinding.h"
#include "../PipelineDefinition.h"
#include "../Pipeline.h"
#include "../Shader.h"
#include "../Palette/Palette.h"

#include "../../Resource/ResourceSystem.h"
#include "../../EngineContext.h"

namespace OpenXcom
{

LineListPrimitive::LineListPrimitive(EngineContext& context, Pipeline& pipeline, RenderTarget& surface,
									 const LineVertex* lines, size_t count, int color, const Palette& palette)
{
	ResourceSystem& resourceSystem = context.getResourceSystem();
	BufferManager& bufferManager = resourceSystem.getBufferManager();

	_pipelineBinding = pipeline.createBinding();

	_vertexBuffer = bufferManager.createDeviceBuffer<LineVertex>(lines, count, BufferUsage::Vertex);
	_pipelineBinding->setVertexBuffer(*_vertexBuffer);

	_pipelineBinding->setUniformBuffer(ShaderStage::Vertex, 0, palette.getDeviceBuffer());

	LinePushConstants pushConstants = {glm::ivec2(surface.getExtent()), color};
	_pipelineBinding->setPushConstant(ShaderStage::Vertex, pushConstants);
}

LineListPrimitive::~LineListPrimitive()
{
}

void LineListPrimitive::draw(GraphicsCommand& command)
{
	_pipelineBinding->commit(command);
}

LineStripPrimitive::LineStripPrimitive(Pipeline& pipeline, RenderTarget& surface)
{
	_pipelineBinding = pipeline.createBinding();
}

LineStripPrimitive::~LineStripPrimitive()
{
}


} // namespace OpenXcom
