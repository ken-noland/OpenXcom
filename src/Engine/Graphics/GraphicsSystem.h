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
#include <memory>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace OpenXcom
{

class Options;
class EngineContext;

enum class ImageFormat;
class GraphicsSurface;
class RenderTarget;

class PlatformWindow;

class ShaderManager;
class PipelineManager;
class BufferManager;
class ImageManager;

class PipelineDefinition;

class GraphicsSystem
{
public:
	GraphicsSystem() = default;
	virtual ~GraphicsSystem() = default;

	virtual std::unique_ptr<GraphicsSurface> createWindowedSurface(PlatformWindow& window) = 0;
	virtual std::unique_ptr<GraphicsSurface> createHeadlessSurface() = 0;
	virtual std::unique_ptr<RenderTarget> createRenderTarget(glm::ivec2 size, ImageFormat format, glm::vec4 color) = 0;

	// resource managers
	virtual std::unique_ptr<ShaderManager> createShaderManager() = 0;
	virtual std::unique_ptr<PipelineManager> createPipelineManager() = 0;
	virtual std::unique_ptr<BufferManager> createBufferManager() = 0;
	virtual std::unique_ptr<ImageManager> createImageManager() = 0;
};

std::unique_ptr<GraphicsSystem> createGraphicsSystem(EngineContext& options);

} // namespace OpenXcom
