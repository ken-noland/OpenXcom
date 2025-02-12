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

#include "../Font/Font.h"
#include "../Font/FontManager.h"

#include "../../EngineContext.h"
#include "../../Resource/ResourceSystem.h"

#include <linebreak.h>


#include "../../Utility/RTTR.h"

SIMPLERTTR
{
	SimpleRTTR::registration().type<OpenXcom::TextVertex>()
		.property(&OpenXcom::TextVertex::position, "position")
		.property(&OpenXcom::TextVertex::texCoord, "texCoord");
}



namespace OpenXcom
{

TextPrimitive::TextPrimitive(EngineContext& context, Pipeline& pipeline, RenderTarget& surface, const std::string text, const TextSettings& settings)
	: _context(context), _settings(settings)
{
	setText(text);
}

TextPrimitive::~TextPrimitive()
{
}

void TextPrimitive::setText(const std::string& text)
{
	// Nothing to do if the text is the same
	if (_text == text)
	{
		return;
	}

	_text = text;

	// Step 1: Cut up the text by any style delimiters(ANSI escape codes)
	_sections = parseTextSections();

	// Step 2: Process line wrapping
	_lines = wrapText();

	// Step 3: Generate glyphs
	generateGlyphs();

}

void TextPrimitive::draw(GraphicsCommand& command)
{
}

std::vector<TextSection> TextPrimitive::parseTextSections()
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	FontManager& fontManager = resourceSystem.getFontManager();

	std::vector<TextSection> sections;
	ResourceHandle<Font> currentFont = _settings.defaultFontHandle;
	TextStyle currentStyle{0,0,false};

	// Use the member _text as the source string.
	const std::string& rawText = _text;

	std::string::const_iterator it = rawText.begin();
	std::string::const_iterator end = rawText.end();
	// sectionStart marks the beginning of a plain-text segment.
	std::string::const_iterator sectionStart = it;

	while (it != end)
	{
		if (*it == '\x1B') // ESC character found.
		{
			// Flush any plain text from sectionStart up to (but not including) this ESC.
			if (it != sectionStart)
			{
				std::string_view view(&*sectionStart, static_cast<size_t>(it - sectionStart));
				sections.emplace_back(TextSection{view, currentStyle, currentFont});
			}

			// Verify that the character following ESC is '['.
			std::string::const_iterator nextIt = std::next(it);
			if (nextIt != end && *nextIt == '[')
			{
				// Skip the ESC and '['.
				std::string::const_iterator seqStart = std::next(it, 2);
				std::string parameter;
				char terminator = '\0';
				std::string::const_iterator j = seqStart;

				// Read characters until a letter (the terminator) is encountered.
				while (j != end && !std::isalpha(static_cast<unsigned char>(*j)))
				{
					parameter.push_back(*j);
					++j;
				}
				if (j != end)
				{
					terminator = *j;
				}
				else
				{
					break; // Incomplete escape sequence.
				}

				// Process the escape sequence.
				if (terminator == 'f')
				{
					const std::string fnPrefix = "fn=";
					if (parameter.substr(0, fnPrefix.size()) == fnPrefix)
					{
						std::string fontName = parameter.substr(fnPrefix.size());
						currentFont = fontManager.getFontByName(fontName);
					}
				}
				else if (terminator == 'm')
				{
					if (parameter.empty())
					{
						// Reset style and font.
						currentStyle = TextStyle();
						currentFont = _settings.defaultFontHandle;
					}
					else
					{
						std::istringstream iss(parameter);
						std::string token;
						while (std::getline(iss, token, ';'))
						{
							int code = std::stoi(token);
							if (code == 0)
							{
								currentStyle = TextStyle();
								currentFont = _settings.defaultFontHandle;
							}
							else if (code == 1)
							{
								// Bold code – add handling if your TextStyle supports it.
							}
							else if (code >= 30 && code <= 37)
							{
								currentStyle.colorIndex = static_cast<uint8_t>(code - 30);
							}
							else if (code == 39)
							{
								currentStyle.colorIndex = TextStyle().colorIndex;
							}
							else if (code >= 40 && code <= 47)
							{
								currentStyle.backgroundColorIndex = static_cast<uint8_t>(code - 40);
							}
							else if (code == 49)
							{
								currentStyle.backgroundColorIndex = TextStyle().backgroundColorIndex;
							}
							else if (code == 4)
							{
								currentStyle.underline = true;
							}
							else if (code == 24)
							{
								currentStyle.underline = false;
							}
							// Add additional SGR codes as needed.
						}
					}
				}
				// Advance the main iterator to the character after the terminator.
				it = std::next(j);
				// Set sectionStart for the next plain-text segment.
				sectionStart = it;
				continue; // Continue with the next character.
			}
			else
			{
				// Not a valid escape sequence; treat the ESC as normal text.
				++it;
			}
		}
		else
		{
			// Regular character: simply advance.
			++it;
		}
	}

	// Flush any remaining text after the loop.
	if (it != sectionStart)
	{
		std::string_view view(&*sectionStart, static_cast<size_t>(it - sectionStart));
		sections.emplace_back(TextSection{view, currentStyle, currentFont});
	}

	return sections;
}

std::vector<TextLine> TextPrimitive::wrapText()
{
	std::vector<TextLine> lines;

	int maxWidth = _settings.extents.x;
	int y_offset = _settings.offset.y;

	std::string_view textView(_text);

	// lineStartIdx is an index into _text marking where the current line begins.
	size_t lineStartIdx = 0;
	// currentLineWidth accumulates the measured width (in pixels) of the current line.
	int currentLineWidth = 0;
	// currentLineHeight is the maximum line height (from any font encountered on this line).
	int currentLineHeight = 0;
	// globalIndex tracks our overall progress through _text.
	size_t globalIndex = 0;

	ResourceSystem& resourceSystem = _context.getResourceSystem();
	FontManager& fontManager = resourceSystem.getFontManager();

	// Iterate over each section. Each section.text is a view into _text.
	for (const TextSection& section : _sections)
	{
		// Get the font for this section.
		Font& font = fontManager.get(section.font);

		// Precompute break properties for this section.
		size_t secLen = section.text.size();
		std::vector<char> breakProps(secLen);
		set_linebreaks_utf8(reinterpret_cast<const utf8_t*>(section.text.data()),
							secLen, "en", breakProps.data());

		// For this section, track the last allowed break (global index) and the width at that break.
		size_t lastAllowedBreak = std::string_view::npos;
		int lastAllowedBreakWidth = 0;

		// Process each character in this section.
		for (size_t i = 0; i < secLen; ++i)
		{
			char flag = breakProps[i];

			// (Measure this character’s width using the section’s font.)
			// We assume here that calling getTextExtents on a one-character view is acceptable.
			std::string_view curChar = section.text.substr(i, 1);
			glm::ivec2 extents = font.getTextExtents(curChar);
			currentLineHeight = std::max(currentLineHeight, extents.y);

			currentLineWidth += extents.x;

			// If this character marks an allowed break, record its global position and the current width.
			if (flag == LINEBREAK_ALLOWBREAK)
			{
				lastAllowedBreak = globalIndex + i + 1; // break comes after this character
				lastAllowedBreakWidth = currentLineWidth;
			}

			// If this character is a mandatory break (e.g. an explicit newline), flush the line immediately.
			if (flag == LINEBREAK_MUSTBREAK)
			{
				size_t breakPos = globalIndex + i + 1;
				std::string_view lineView = textView.substr(lineStartIdx, breakPos - lineStartIdx);
				lines.push_back({lineView, y_offset, currentLineHeight});
				y_offset += currentLineHeight;
				// Start a new line after the break.
				lineStartIdx = breakPos;
				currentLineWidth = 0;
				currentLineHeight = 0;
				// Reset allowed break tracking.
				lastAllowedBreak = std::string_view::npos;
				lastAllowedBreakWidth = 0;
				continue;
			}

			// Check if adding this character makes the current line too wide.
			if (currentLineWidth > maxWidth)
			{
				// If we have a recorded allowed break, use that.
				if (lastAllowedBreak != std::string_view::npos && lastAllowedBreak > lineStartIdx)
				{
					std::string_view lineView = textView.substr(lineStartIdx, lastAllowedBreak - lineStartIdx);
					lines.push_back({lineView, y_offset, currentLineHeight});
					y_offset += currentLineHeight;
					// Start the new line at the allowed break.
					lineStartIdx = lastAllowedBreak;
					// The new line’s width is the overflow beyond the allowed break.
					currentLineWidth = currentLineWidth - lastAllowedBreakWidth;
				}
				// Otherwise, force-break at the current character.
				else
				{
					size_t breakPos = globalIndex + i + 1;
					std::string_view lineView = textView.substr(lineStartIdx, breakPos - lineStartIdx);
					lines.push_back({lineView, y_offset, currentLineHeight});
					y_offset += currentLineHeight;
					lineStartIdx = breakPos;
					currentLineWidth = 0;
				}
				// Reset allowed break tracking for the new line.
				lastAllowedBreak = std::string_view::npos;
				lastAllowedBreakWidth = 0;
			}
		}
		// Advance the global index by the length of this section.
		globalIndex += secLen;
	}

	// Flush any remaining text on the final line.
	if (globalIndex > lineStartIdx)
	{
		std::string_view lineView = textView.substr(lineStartIdx, globalIndex - lineStartIdx);
		lines.push_back({lineView, y_offset, currentLineHeight});
	}
	return lines;
}

void TextPrimitive::generateGlyphs()
{
	//for(TextSection& section : _sections)
	//{
	//	const Font& font = _context.getResourceSystem().getFontManager().get(section.font);
	//	std::string temp(section.text.begin(), section.text.end());
	//	section.glyphs = font.shapeText(temp, glm::ivec2(0, 0));
	//}
}

} // namespace OpenXcom
