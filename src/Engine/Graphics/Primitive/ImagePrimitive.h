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
#include <glm/vec2.hpp>

namespace OpenXcom
{

class EngineContext;
class GraphicsCommand;
class PipelineBinding;
class Pipeline;
class RenderTarget;
class HostBuffer;
class DeviceBuffer;
class DeviceImage;
class Palette;

template <typename ResourceType>
class ResourceHandle;

struct ImageVertex
{
	glm::ivec2 pos;
	glm::ivec2 uv;
};

class ImagePrimitive : public Primitive
{
protected:
	EngineContext& _context;

	std::unique_ptr<PipelineBinding> _pipelineBinding;

	std::unique_ptr<HostBuffer> _vertexHostBuffer;
	std::unique_ptr<DeviceBuffer> _vertexDeviceBuffer;

public:
	ImagePrimitive(EngineContext& context, Pipeline& pipeline, RenderTarget& surface,
				   glm::ivec2 dstPosition, glm::ivec2 srcPosition, glm::ivec2 size,
				   const ResourceHandle<DeviceImage>& imageHandle, const ResourceHandle<Palette>& paletteHandle);
	virtual ~ImagePrimitive();

	void set(glm::ivec2 dstPosition, glm::ivec2 srcPosition, glm::ivec2 extents);

	void setPalette(const ResourceHandle<Palette>& paletteHandle);
	void setImage(const ResourceHandle<DeviceImage>& imageHandle);
	void setSurface(RenderTarget& surface);

	void draw(GraphicsCommand& command);
};

} // namespace OpenXcom
