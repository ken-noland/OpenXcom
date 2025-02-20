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

namespace OpenXcom
{

class EngineContext;
class RenderTarget;

class Pipeline;
class Palette;
class ShaderCollection;

template <typename ResourceType>
class ResourceHandle;

class LineListPrimitive;
class LineStripPrimitive;

struct LineVertex;

class LinePrimitiveFactory
{
protected:
	EngineContext& _context;
	RenderTarget& _surface;

	std::unique_ptr<Pipeline> _lineListPipeline;
	std::unique_ptr<Pipeline> _lineStripPipeline;

public:
	LinePrimitiveFactory(EngineContext& context, RenderTarget& surface, ShaderCollection& shaders);
	~LinePrimitiveFactory();

	std::unique_ptr<LineListPrimitive> createLineListPrimitive(const LineVertex* lines, size_t count, int color, const ResourceHandle<Palette>& paletteHandle);	
	std::unique_ptr<LineStripPrimitive> createLineStripPrimitive(const LineVertex* lines, size_t count, int color, const ResourceHandle<Palette>& paletteHandle);
};

} // namespace OpenXcom
