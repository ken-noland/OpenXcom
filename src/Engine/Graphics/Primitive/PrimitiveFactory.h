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
#include "LinePrimitiveFactory.h"
#include <memory>

namespace OpenXcom
{

class EngineContext;
class LineListPrimitive;
class LineStripPrimitive;
class Palette;

class PrimitiveFactory
{
protected:
	LinePrimitiveFactory _lineFactory;

public:
	PrimitiveFactory(EngineContext& context, RenderTarget& surface);
	~PrimitiveFactory();

	std::unique_ptr<LineListPrimitive> createLineListPrimitive(const LineVertex* lines, size_t count, int color, const Palette& palette); 
	std::unique_ptr<LineStripPrimitive> createLineStripPrimitive();
};

} // namespace OpenXcom
