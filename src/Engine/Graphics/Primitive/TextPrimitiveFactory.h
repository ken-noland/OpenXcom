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
#include <string>

namespace OpenXcom
{

class EngineContext;
class RenderTarget;

class Pipeline;
class Palette;
class Font;
class ShaderCollection;

template <typename ResourceType>
class ResourceHandle;

class TextPrimitive;
struct TextSettings;

class TextPrimitiveFactory
{
protected:
	EngineContext& _context;
	RenderTarget& _surface;
	ShaderCollection& _shaders;

	std::unique_ptr<Pipeline> _textPipeline;

public:
	TextPrimitiveFactory(EngineContext& context, RenderTarget& surface, ShaderCollection& shaders);
	~TextPrimitiveFactory();

	std::unique_ptr<TextPrimitive> createTextPrimitive(const std::string& text, const TextSettings& setting);
};

} // namespace OpenXcom
