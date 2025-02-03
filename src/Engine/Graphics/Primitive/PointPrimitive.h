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
#include <array>
#include <memory>
#include <vector>

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

struct PointVertex
{
	glm::ivec2 pos;
};

struct PointColorVertex
{
	glm::ivec2 pos;
	int color;
};

struct PointPushConstants
{
	glm::ivec2 surfaceExtent; // TODO: temporary until I move the screen extents to a uniform buffer
	int color;
};

struct PointColorPushConstants
{
	glm::ivec2 surfaceExtent; // TODO: temporary until I move the screen extents to a uniform buffer
};

class PointListPrimitive : public Primitive
{
protected:
	std::unique_ptr<PipelineBinding> _pipelineBinding;
	std::unique_ptr<DeviceBuffer> _vertexBuffer;

public:
	PointListPrimitive(EngineContext& context, Pipeline& pipeline, RenderTarget& surface,
					   const PointVertex* points, size_t count, int color, const ResourceHandle<Palette>& paletteHandle);
	virtual ~PointListPrimitive();

	void draw(GraphicsCommand& command);
};

class PointColorListPrimitive : public Primitive
{
protected:
	std::unique_ptr<PipelineBinding> _pipelineBinding;
	std::unique_ptr<DeviceBuffer> _vertexBuffer;

public:
	PointColorListPrimitive(EngineContext& context, Pipeline& pipeline, RenderTarget& surface,
							const PointColorVertex* points, size_t count, const ResourceHandle<Palette>& paletteHandle);
	virtual ~PointColorListPrimitive();

	void draw(GraphicsCommand& command);
};

} // namespace OpenXcom
