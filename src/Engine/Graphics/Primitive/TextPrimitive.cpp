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

#include "../../Logger.h"

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
	processTextSections();

	// Step 2: Process line shaping(word wrapping)
	processLineShaping();

	// Step 3: Generate glyphs(text alignment and placement)
	generateGlyphs();

	// Step 4: Generate vertices

}

void TextPrimitive::draw(GraphicsCommand& command)
{
}

void TextPrimitive::processTextSections()
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	FontManager& fontManager = resourceSystem.getFontManager();

	_sections.clear();
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
				_sections.emplace_back(TextSection{view, currentStyle, currentFont});
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
		_sections.emplace_back(TextSection{view, currentStyle, currentFont});
	}
}

void TextPrimitive::processLineShaping()
{
	_lines.clear();

	int maxWidth = _settings.extents.x;
	int y_offset = _settings.offset.y;
	std::string_view textView(_text);
	
	if (maxWidth <= 0)
	{
		// No line wrapping, so insert a single line with the entire text
		_lines.push_back({textView, y_offset, 0});
		return;
	}

	// Global index into _text: where the current line starts.
	size_t lineStartIdx = 0;
	// Width accumulated from previous sections on the current line.
	int accumulatedWidth = 0;
	// The maximum line height seen so far.
	int accumulatedHeight = 0;
	// Global progress through _text.
	size_t globalIndex = 0;

	ResourceSystem& resourceSystem = _context.getResourceSystem();
	FontManager& fontManager = resourceSystem.getFontManager();

	// For allowed-break tracking over the current line:
	size_t allowedBreakGlobal = std::string_view::npos; // global index (into _text)
	int allowedBreakLineWidth = 0;                      // measured width at that allowed break
	// For the current section, track the local run start (in section.text)
	// where the current run begins (for kerning purposes).
	size_t localRunStart = 0;
	// For allowed break within the current section.
	size_t localAllowedBreak = std::string_view::npos;

	// For tracking the extents of the current run in a line.
	glm::ivec2 runExtents(0,0);

	// For tracking the extents of the previous character in a line. (used when a word extends beyond the max width)
	glm::ivec2 prevExtents(0, 0);

	// Process each section.
	for (const TextSection& section : _sections)
	{
		Font& font = fontManager.get(section.font);

		// Precompute break properties for this section.
		size_t secLen = section.text.size();
		std::vector<char> breakProps(secLen);
		set_linebreaks_utf8(reinterpret_cast<const utf8_t*>(section.text.data()),
							secLen, "en", breakProps.data());

		// Reset the current section’s local-run start.
		localRunStart = 0;
		localAllowedBreak = std::string_view::npos;

		for (size_t i = 0; i < secLen; ++i)
		{
			//Log(LOG_DEBUG) << "Processing character: '" << section.text[i] << "'(index " << i << ")";

			char flag = breakProps[i];

			prevExtents = runExtents;

			// Measure the run from localRunStart to i (in this section)
			std::string_view runSegment = section.text.substr(localRunStart, i - localRunStart + 1);
			runExtents = font.getTextExtents(runSegment);
			// The current line width is the sum of the accumulated width (from previous sections)
			// and the measured width in the current section’s run.
			int currentLineWidth = accumulatedWidth + runExtents.x;

			//Log(LOG_DEBUG) << "Current line width: " << currentLineWidth;

			// Update line height from this section’s font (using a test string like "Ay")
			int currentLineHeight = std::max(accumulatedHeight, runExtents.y);
			
			if (flag == LINEBREAK_ALLOWBREAK)
			{
				// If we hit an allowed break, record it.
				//Log(LOG_DEBUG) << "Allowed break at index: " << i;

				localAllowedBreak = i + 1; // break comes after this character in the current section
				allowedBreakGlobal = globalIndex + localAllowedBreak;
				allowedBreakLineWidth = accumulatedWidth + prevExtents.x;// use the previous extents to ignore the wrapping character

				//Log(LOG_DEBUG) << "Allowed break index: " << allowedBreakGlobal;
				//Log(LOG_DEBUG) << "Allowed break width: " << allowedBreakLineWidth;

			}			
			else if (flag == LINEBREAK_MUSTBREAK)
			{
				// On a mandatory break, flush immediately.
				size_t breakPos = globalIndex + i + 1;
				std::string_view lineView = textView.substr(lineStartIdx, breakPos - lineStartIdx);
				_lines.push_back({lineView, y_offset, currentLineHeight, currentLineWidth});

				accumulatedHeight = currentLineHeight;
				y_offset += currentLineHeight;

				// Reset everything for the new line.
				lineStartIdx = breakPos;
				accumulatedWidth = 0;
				localRunStart = i + 1; // start new run in this section after the break
				allowedBreakGlobal = std::string_view::npos;
				localAllowedBreak = std::string_view::npos;
				continue;
			}

			// Check if the current line (accumulated width + current run width) is too wide.
			if (currentLineWidth > maxWidth)
			{
				//Log(LOG_DEBUG) << "Line too wide; current width: " << currentLineWidth << ", max width: " << maxWidth;
				//Log(LOG_DEBUG) << "Last allowed break index: " << allowedBreakGlobal;

				// If we have a recorded allowed break (and it comes after the current line start)
				if (allowedBreakGlobal != std::string_view::npos && allowedBreakGlobal > lineStartIdx)
				{
					// Flush the line at the allowed break.
					std::string_view lineView = textView.substr(lineStartIdx, allowedBreakGlobal - lineStartIdx);
					_lines.push_back({lineView, y_offset, currentLineHeight, allowedBreakLineWidth});

					accumulatedHeight = currentLineHeight;
					y_offset += currentLineHeight;

					// Start new line at allowed break.
					lineStartIdx = allowedBreakGlobal;
					// The new line's accumulated width is the overflow from the allowed break.
					// That is, current width minus the width at the allowed break.
					accumulatedWidth = 0;
					// In the current section, set the local run start to the allowed break position.
					localRunStart = localAllowedBreak;
				}
				else
				{
					// Otherwise, force a break at the current character.
					size_t breakPos = globalIndex + i;
					std::string_view lineView = textView.substr(lineStartIdx, breakPos - lineStartIdx);
					_lines.push_back({lineView, y_offset, currentLineHeight, prevExtents.x});

					accumulatedHeight = currentLineHeight;
					y_offset += currentLineHeight;

					lineStartIdx = breakPos;
					accumulatedWidth = 0;
					localRunStart = i;
				}
				// Reset allowed break tracking for the new line.
				allowedBreakGlobal = std::string_view::npos;
				localAllowedBreak = std::string_view::npos;
			}
		} // end for section's characters

		// At the end of the section, add the width of any remaining run from localRunStart.
		if (secLen > localRunStart)
		{
			std::string_view remainder = section.text.substr(localRunStart);
			accumulatedWidth += font.getTextExtents(remainder).x;
		}
		globalIndex += secLen;
	}

	// Flush any remaining text as the final line.
	if (globalIndex > lineStartIdx)
	{
		std::string_view lineView = textView.substr(lineStartIdx, globalIndex - lineStartIdx);
		_lines.push_back({lineView, y_offset, accumulatedHeight, accumulatedWidth});
	}
}

std::string_view intersectViews(std::string_view a, std::string_view b)
{
	const char* a_start = a.data();
	const char* a_end = a_start + a.size();
	const char* b_start = b.data();
	const char* b_end = b_start + b.size();

	// The intersection starts at the later of the two start pointers...
	const char* inter_start = std::max(a_start, b_start);
	// ...and ends at the earlier of the two end pointers.
	const char* inter_end = std::min(a_end, b_end);

	if (inter_start < inter_end)
		return std::string_view(inter_start, inter_end - inter_start);
	else
		return std::string_view(); // empty view if no overlap
}

void TextPrimitive::generateGlyphs()
{
	assert(!_lines.empty());

	std::vector<TextLine>::iterator lineIter = _lines.begin();

	glm::ivec2 position = _settings.offset;

	// For each section
	for(TextSection& section : _sections)
	{
		Font& font = _context.getResourceSystem().getFontManager().get(section.font);

		// Get the intersection of the section and the line
		for (std::string_view intersection = intersectViews(section.text, lineIter->line);
			 intersection.length() != 0;
			 intersection = intersectViews(section.text, lineIter->line))
		{
			std::vector<PositionedGlyph> glyphs = font.shapeText(intersection, position);
			section.glyphs.insert(section.glyphs.end(), glyphs.begin(), glyphs.end());

			bool endOfSection = intersection.data() + intersection.size() == section.text.data() + section.text.size();
			bool endOfLine = intersection.data() + intersection.size() == lineIter->line.data() + lineIter->line.size();

			// if we are at the end of a section and end of a line, then move on to the next line
			if (endOfSection && endOfLine)
			{
				position.y += lineIter->yHeight;
				position.x = _settings.offset.x;
				++lineIter;

				if (lineIter == _lines.end())
				{
					return;
				}

				break;
			}

			// if we are at the end of the section, then move on to the next section
			if (endOfSection)
			{
				// update position based on the last glyph
				PositionedGlyph& lastGlyph = glyphs.back();
				position.x = lastGlyph.position.x + lastGlyph.advance;

				break;
			}

			// if we are at the end of the line, then move on to the next line
			if (endOfLine)
			{
				position.y += lineIter->yHeight;
				position.x = _settings.offset.x;
				++lineIter;

				if(lineIter == _lines.end())
				{
					// Done
					return;
				}
			}
		}
	}
}

} // namespace OpenXcom
