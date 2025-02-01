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
#include "PrimitiveFactory.h"
#include "BoxPrimitive.h"
#include "LinePrimitive.h"

namespace OpenXcom
{

PrimitiveFactory::PrimitiveFactory(EngineContext& context, RenderTarget& surface)
	: _lineFactory(context, surface), _boxFactory(context, surface)
{
}

PrimitiveFactory::~PrimitiveFactory()
{
}

std::unique_ptr<BoxFilledPrimitive> PrimitiveFactory::createFilledBoxPrimitive(glm::ivec2 position, glm::ivec2 size, int color, const ResourceHandle<Palette>& paletteHandle)
{
	return _boxFactory.createFilledBoxPrimitive(position, size, color, paletteHandle);
}

std::unique_ptr<BoxOutlinePrimitive> PrimitiveFactory::createOutlineBoxPrimitive(glm::ivec2 position, glm::ivec2 size, int color, const ResourceHandle<Palette>& palette)
{
	return _boxFactory.createOutlineBoxPrimitive(position, size, color, palette);
}

std::unique_ptr<LineListPrimitive> PrimitiveFactory::createLineListPrimitive(const LineVertex* lines, size_t count, int color, const ResourceHandle<Palette>& palette)
{
	return _lineFactory.createLineListPrimitive(lines, count, color, palette);
}

std::unique_ptr<LineStripPrimitive> PrimitiveFactory::createLineStripPrimitive(const LineVertex* lines, size_t count, int color, const ResourceHandle<Palette>& palette)
{
	return _lineFactory.createLineStripPrimitive(lines, count, color, palette);
}

} // namespace OpenXcom
