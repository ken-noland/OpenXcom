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
#include "TextPrimitive.h"
#include "../Buffer/Buffer.h"
#include "../PipelineBinding.h"

namespace OpenXcom
{

TextPrimitive::TextPrimitive(EngineContext& context, Pipeline& pipeline, RenderTarget& surface, const std::string text, const ResourceHandle<Font>& defaultFontHandle, const ResourceHandle<Palette>& paletteHandle)
	: _context(context)
{
}

TextPrimitive::~TextPrimitive()
{
}

void TextPrimitive::setText(const std::string& text)
{
	// Step 1: Cut up the text by any style delimiters(ANSI escape codes)
}

void TextPrimitive::draw(GraphicsCommand& command)
{
}

} // namespace OpenXcom
