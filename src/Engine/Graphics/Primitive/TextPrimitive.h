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

#include "../Font/Font.h"

#include <array>
#include <memory>
#include <vector>
#include <string>

#include <glm/vec2.hpp>

namespace OpenXcom
{

class EngineContext;
class GraphicsCommand;
class PipelineBinding;
class Pipeline;
class RenderTarget;
class DeviceBuffer;
class Palette;
class Font;

template <typename ResourceType>
class ResourceHandle;

enum class TextAlignment
{
	Left,
	Center,
	Right
};


struct TextVertex
{
	glm::ivec2 position;
	glm::ivec2 texCoord;
};

struct TextStyle
{
	uint8_t colorIndex;
	uint8_t backgroundColorIndex;
	bool underline;
};

struct TextSettings
{
	TextAlignment alignment;

	glm::ivec2 extents;
	glm::ivec2 offset;

	ResourceHandle<Font> defaultFontHandle;
	ResourceHandle<Palette> defaultPaletteHandle;
};

struct TextSection
{
	std::string_view text;
	TextStyle style;
	ResourceHandle<Font> font;

	std::vector<PositionedGlyph> glyphs;
};

struct TextLine
{
	std::string_view line;

	int32_t yOffset;
	int32_t yHeight;

	int32_t lineWidth;

	TextAlignment alignment;
};

class TextPrimitive : public Primitive
{
protected:
	EngineContext& _context;

	std::string _text;
	TextSettings _settings;

	std::vector<TextSection> _sections;
	std::vector<TextLine> _lines;
	std::vector<PositionedGlyph> _glyphs;

	std::unique_ptr<PipelineBinding> _pipelineBinding;
	std::unique_ptr<DeviceBuffer> _vertexBuffer;

	void processTextSections();
	void processLineShaping();
	void generateGlyphs();

public:
	TextPrimitive(EngineContext& context, Pipeline& pipeline, RenderTarget& surface,
				  const std::string text, const TextSettings& settings);
	virtual ~TextPrimitive();

	void setText(const std::string& text);

	const std::vector<TextSection>& getSections() const { return _sections; }
	const std::vector<TextLine>& getLines() const { return _lines; }

	void draw(GraphicsCommand& command);
};

} // namespace OpenXcom
