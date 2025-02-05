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
class Palette;


template <typename ResourceType>
class ResourceHandle;

struct LineVertex
{
	glm::ivec2 pos;
};

struct LinePushConstants
{
	int color;
};

class LineListPrimitive : public Primitive
{
protected:
	EngineContext& _context;

	std::unique_ptr<PipelineBinding> _pipelineBinding;
	std::unique_ptr<DeviceBuffer> _vertexBuffer;

public:
	LineListPrimitive(EngineContext& context, Pipeline& pipeline, RenderTarget& surface,
					  const LineVertex* lines, size_t count, int color, const ResourceHandle<Palette>& paletteHandle);
	virtual ~LineListPrimitive();

	void setLines(const LineVertex* lines, size_t count);
	void setPaletteColorIndex(int color);
	void setPalette(const ResourceHandle<Palette>& paletteHandle);
	void setSurface(RenderTarget& surface);

	void draw(GraphicsCommand& command);
};

class LineStripPrimitive : public Primitive
{
protected:
	EngineContext& _context;

	std::unique_ptr<PipelineBinding> _pipelineBinding;
	std::unique_ptr<DeviceBuffer> _vertexBuffer;

public:
	LineStripPrimitive(EngineContext& context, Pipeline& pipeline, RenderTarget& surface,
					   const LineVertex* lines, size_t count, int color, const ResourceHandle<Palette>& paletteHandle);
	virtual ~LineStripPrimitive();

	void setLines(const LineVertex* lines, size_t count);
	void setPaletteColorIndex(int color);
	void setPalette(const ResourceHandle<Palette>& paletteHandle);
	void setSurface(RenderTarget& surface);

	void draw(GraphicsCommand& command);
};

} // namespace OpenXcom
