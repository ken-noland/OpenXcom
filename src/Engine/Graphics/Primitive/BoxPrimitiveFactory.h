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

namespace OpenXcom
{

class EngineContext;
class RenderTarget;

class Pipeline;
class Shader;
class Palette;

template <typename ResourceType>
class ResourceHandle;

class BoxFilledPrimitive;
class BoxOutlinePrimitive;

struct BoxVertex;

class BoxPrimitiveFactory
{
protected:
	EngineContext& _context;
	RenderTarget& _surface;

	std::unique_ptr<Pipeline> _boxFilledPipeline;
	std::unique_ptr<Pipeline> _boxOutlinePipeline;

	std::unique_ptr<Shader> _vertexShader;
	std::unique_ptr<Shader> _fragmentShader;

public:
	BoxPrimitiveFactory(EngineContext& context, RenderTarget& surface);
	~BoxPrimitiveFactory();

	std::unique_ptr<BoxFilledPrimitive> createFilledBoxPrimitive(glm::ivec2 position, glm::ivec2 size, int color, const ResourceHandle<Palette>& paletteHandle);
	std::unique_ptr<BoxOutlinePrimitive> createOutlineBoxPrimitive(glm::ivec2 position, glm::ivec2 size, int color, const ResourceHandle<Palette>& palette);
};

} // namespace OpenXcom
