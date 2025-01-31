#pragma once
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
#include "Primitive.h"
#include <memory>
#include <vector>
#include <array>

#include <glm/vec2.hpp>

namespace OpenXcom
{

class EngineContext;
class GraphicsCommand;
class PipelineBinding;
class Pipeline;
class RenderTarget;
class DeviceBuffer;

struct LineVertex
{
	glm::ivec2 pos;
};

struct LinePushConstants
{
	glm::ivec2 surfaceExtent;	//TODO: temporary until I move the screen extents to a uniform buffer
	int color;
};

class LineListPrimitive : public Primitive
{
protected:
	std::unique_ptr<PipelineBinding> _pipelineBinding;
	std::unique_ptr<DeviceBuffer> _vertexBuffer;

public:
	LineListPrimitive(EngineContext& context, Pipeline& pipeline, RenderTarget& surface,
					  const LineVertex* lines, size_t count, int color, /* temp */ DeviceBuffer& palette);
	virtual ~LineListPrimitive();

	void draw(GraphicsCommand& command);
};

class LineStripPrimitive : public Primitive
{
protected:
	std::unique_ptr<PipelineBinding> _pipelineBinding;

public:
	LineStripPrimitive(Pipeline& pipeline, RenderTarget& surface);
	virtual ~LineStripPrimitive();
};

} // namespace OpenXcom
