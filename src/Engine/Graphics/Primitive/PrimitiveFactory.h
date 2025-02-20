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
#include "BoxPrimitiveFactory.h"
#include "LinePrimitiveFactory.h"
#include "PointPrimitiveFactory.h"
#include "ImagePrimitiveFactory.h"
#include "TextPrimitiveFactory.h"

#include "ShaderCollection.h"

#include <memory>

namespace OpenXcom
{

class PrimitiveFactory
{
protected:
	ShaderCollection _shaders;

	BoxPrimitiveFactory _boxFactory;
	LinePrimitiveFactory _lineFactory;
	PointPrimitiveFactory _pointFactory;
	ImagePrimitiveFactory _imageFactory;
	TextPrimitiveFactory _textFactory;

public:
	PrimitiveFactory(EngineContext& context, RenderTarget& surface);
	~PrimitiveFactory();

	std::unique_ptr<BoxFilledPrimitive> createFilledBoxPrimitive(glm::ivec2 position, glm::ivec2 size, int color, const ResourceHandle<Palette>& paletteHandle);
	std::unique_ptr<BoxOutlinePrimitive> createOutlineBoxPrimitive(glm::ivec2 position, glm::ivec2 size, int color, const ResourceHandle<Palette>& palette);

	std::unique_ptr<LineListPrimitive> createLineListPrimitive(const LineVertex* lines, size_t count, int color, const ResourceHandle<Palette>& paletteHandle); 
	std::unique_ptr<LineStripPrimitive> createLineStripPrimitive(const LineVertex* lines, size_t count, int color, const ResourceHandle<Palette>& palette);

	std::unique_ptr<PointListPrimitive> createPointListPrimitive(const PointVertex* lines, size_t count, int color, const ResourceHandle<Palette>& paletteHandle);
	std::unique_ptr<PointColorListPrimitive> createPointColorListPrimitive(const PointColorVertex* lines, size_t count, const ResourceHandle<Palette>& paletteHandle);

	std::unique_ptr<ImagePrimitive> createImagePrimitive(glm::ivec2 dst, glm::ivec2 src, glm::ivec2 extents, const ResourceHandle<DeviceImage>& image, const ResourceHandle<Palette>& palette);

	std::unique_ptr<TextPrimitive> createTextPrimitive(const std::string& text, const TextSettings& settings);
};

} // namespace OpenXcom
