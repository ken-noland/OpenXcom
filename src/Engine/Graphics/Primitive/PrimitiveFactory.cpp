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
#include "PointPrimitive.h"
#include "ImagePrimitive.h"
#include "TextPrimitive.h"

namespace OpenXcom
{

PrimitiveFactory::PrimitiveFactory(EngineContext& context, RenderTarget& surface)
	: _shaders(context),
	  _lineFactory(context, surface, _shaders),
	  _boxFactory(context, surface, _shaders),
	  _pointFactory(context, surface, _shaders),
	  _imageFactory(context, surface, _shaders),
	  _textFactory(context, surface, _shaders)
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

std::unique_ptr<PointListPrimitive> PrimitiveFactory::createPointListPrimitive(const PointVertex* lines, size_t count, int color, const ResourceHandle<Palette>& paletteHandle)
{
	return _pointFactory.createPointListPrimitive(lines, count, color, paletteHandle);
}

std::unique_ptr<PointColorListPrimitive> PrimitiveFactory::createPointColorListPrimitive(const PointColorVertex* lines, size_t count, const ResourceHandle<Palette>& paletteHandle)
{
	return _pointFactory.createPointColorListPrimitive(lines, count, paletteHandle);
}

std::unique_ptr<ImagePrimitive> PrimitiveFactory::createImagePrimitive(glm::ivec2 dst, glm::ivec2 src, glm::ivec2 extents, const ResourceHandle<DeviceImage>& image, const ResourceHandle<Palette>& palette)
{
	return _imageFactory.createImagePrimitive(dst, src, extents, image, palette);
}

std::unique_ptr<TextPrimitive> PrimitiveFactory::createTextPrimitive(const std::string& text, const TextSettings& settings)
{
	return _textFactory.createTextPrimitive(text, settings);
}

} // namespace OpenXcom
