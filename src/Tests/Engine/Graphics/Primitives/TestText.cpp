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
#include "../../_TestEngine.h"

#include "../../../../Engine/Graphics/Primitive/TextPrimitive.h"
#include "../../../../Engine/Graphics/Primitive/PrimitiveFactory.h"

#include "../../../../Engine/Graphics/Buffer/BufferManager.h"

#include <filesystem>
#include <memory>

using namespace OpenXcom;

class GraphicsTextTest : public TestEngineSuite
{
protected:
	OwningHandle<Palette> _paletteHandle;
	OwningHandle<Font> _fontHandle;

	void SetUp() override
	{
		_paletteHandle = create16ColorPalette();
		_fontHandle = createDosFont();
	}

	void TearDown() override
	{
		_fontHandle.release();
		_paletteHandle.release();
	}
};

TEST_F(GraphicsTextTest, TestHelloWorld)
{
	std::string text = "Hello World!";

	PrimitiveFactory& factory = _gameSurface->getRenderTarget().getPrimitiveFactory();
	TextSettings settings;

	settings.defaultFontHandle = _fontHandle.getHandle();
	settings.defaultPaletteHandle = _paletteHandle.getHandle();
	settings.extents = {0, 0};
	settings.offset = {0, 0};

	std::unique_ptr<TextPrimitive> textPrimitive = factory.createTextPrimitive(text, settings);
	ASSERT_TRUE(textPrimitive);

	const std::vector<TextLine>& lines = textPrimitive->getLines();
	EXPECT_EQ(1, lines.size());	// 1 line

	const std::vector<TextSection>& sections = textPrimitive->getSections();
	ASSERT_EQ(1, sections.size()); // 1 section

	const TextSection& section = sections[0];
	EXPECT_EQ(text.size(), section.glyphs.size()); // we should have the same number of glyphs as characters in the text
}

TEST_F(GraphicsTextTest, TestColorSections)
{
	//// Using ANSI escape characters to change text color
	//std::string text = "\x1b[31mRed\x1b[0m \x1b[32mGreen\x1b[0m \x1b[34mBlue\x1b[0m";
	//OwningHandle<Font> defaultFont = createDosFont();

	//std::vector<TextSection> sections = parseText(text, defaultFont.getHandle(), _engine->getEngineContext().getResourceSystem().getFontManager());
	throw std::exception("TestColorSections not implemented");
}

TEST_F(GraphicsTextTest, TestHelloWorldShape)
{
	// The text to shape
	std::string text = "Hello World!";

	PrimitiveFactory& factory = _gameSurface->getRenderTarget().getPrimitiveFactory();
	TextSettings settings;

	settings.defaultFontHandle = _fontHandle.getHandle();
	settings.defaultPaletteHandle = _paletteHandle.getHandle();
	settings.extents = {0, 0};
	settings.offset = {0, 0};

	std::unique_ptr<TextPrimitive> textPrimitive = factory.createTextPrimitive(text, settings);
	ASSERT_TRUE(textPrimitive) << "Failed to create text primitive";

	const std::vector<TextSection>& sections = textPrimitive->getSections();
	ASSERT_EQ(1, sections.size()) << "Expected one section";

	const TextSection& section = sections[0];
	ASSERT_EQ(text.size(), section.glyphs.size()) << "Expected one glyph per character";

	// Now verify each glyph's advance and offsets.
	// We expect each glyph's advance to be 9 (since 9 * 64 = 576 in fixed-point)
	// and offsets to be zero.
	uint32_t expectedAdvance = 0;
	for (size_t i = 0; i < section.glyphs.size(); i++)
	{
		const PositionedGlyph& glyph = section.glyphs[i];
		EXPECT_EQ(glyph.advance, 9) << "Glyph index " << i << " has incorrect advance";
		EXPECT_EQ(glyph.position.x, settings.offset.x + expectedAdvance) << "Glyph index " << i << " has incorrect x position";
		EXPECT_EQ(glyph.offsetX, 0) << "Glyph index " << i << " has nonzero x offset";
		EXPECT_EQ(glyph.offsetY, 0) << "Glyph index " << i << " has nonzero y offset";
		expectedAdvance += glyph.advance;
	}
}

TEST_F(GraphicsTextTest, TestMultiLineLoremIpsum)
{
	// The text to shape
	std::string text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
					   "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";

	PrimitiveFactory& factory = _gameSurface->getRenderTarget().getPrimitiveFactory();
	TextSettings settings;

	settings.defaultFontHandle = _fontHandle.getHandle();
	settings.defaultPaletteHandle = _paletteHandle.getHandle();
	settings.extents = {200, 0};
	settings.offset = {0, 0};

	std::unique_ptr<TextPrimitive> textPrimitive = factory.createTextPrimitive(text, settings);
	ASSERT_TRUE(textPrimitive) << "Failed to create text primitive";

	const std::vector<TextSection>& sections = textPrimitive->getSections();
	ASSERT_EQ(1, sections.size()) << "Expected one section";

	const TextSection& section = sections[0];
	ASSERT_EQ(text.size(), section.glyphs.size()) << "Expected one glyph per character";

	// First line is "Lorem ipsum dolor sit "
	EXPECT_EQ(section.glyphs[0].codepoint, 'L');
	EXPECT_EQ(section.glyphs[0].position.x, settings.offset.x);
	EXPECT_EQ(section.glyphs[0].position.y, settings.offset.y + (16 * 0));

	// Second line is "amet, consectetur "
	EXPECT_EQ(section.glyphs[22].codepoint, 'a');
	EXPECT_EQ(section.glyphs[22].position.x, settings.offset.x);
	EXPECT_EQ(section.glyphs[22].position.y, settings.offset.y + (16 * 1));

	// Third line is "adipiscing elit. Sed "
	EXPECT_EQ(section.glyphs[40].codepoint, 'a');
	EXPECT_EQ(section.glyphs[40].position.x, settings.offset.x);
	EXPECT_EQ(section.glyphs[40].position.y, settings.offset.y + (16 * 2));

	// Fourth line is "do eiusmod tempor "
	EXPECT_EQ(section.glyphs[61].codepoint, 'd');
	EXPECT_EQ(section.glyphs[61].position.x, settings.offset.x);
	EXPECT_EQ(section.glyphs[61].position.y, settings.offset.y + (16 * 3));

	// Fifth line is "incididunt ut labore "
	EXPECT_EQ(section.glyphs[79].codepoint, 'i');
	EXPECT_EQ(section.glyphs[79].position.x, settings.offset.x);
	EXPECT_EQ(section.glyphs[79].position.y, settings.offset.y + (16 * 4));

	// Sixth line is "et dolore magna "
	EXPECT_EQ(section.glyphs[100].codepoint, 'e');
	EXPECT_EQ(section.glyphs[100].position.x, settings.offset.x);
	EXPECT_EQ(section.glyphs[100].position.y, settings.offset.y + (16 * 5));

	// Seventh line is "aliqua."
	EXPECT_EQ(section.glyphs[116].codepoint, 'a');
	EXPECT_EQ(section.glyphs[116].position.x, settings.offset.x);
	EXPECT_EQ(section.glyphs[116].position.y, settings.offset.y + (16 * 6));

	EXPECT_EQ(section.glyphs[117].codepoint, 'l');
	EXPECT_EQ(section.glyphs[117].position.x, settings.offset.x + 9);
	EXPECT_EQ(section.glyphs[117].position.y, settings.offset.y + (16 * 6));
}

//TEST_F(GraphicsTextTest, TestMultiReallyLongWord)
//{
//	// The text to shape
//	std::string text = "A0000000000000000000B1111111111111111111C2222222222222222222D3333333333333333333";
//
//	// Starting position (for example, top-left corner at (0,0))
//	glm::ivec2 startPos(0, 0);
//
//	// Maximum width for wrapping
//	int maxWidth = 180;
//
//	// Call your Font::wrappedText() method
//	std::vector<PositionedGlyph> positionedGlyphs = _font->wrappedText(text, startPos, maxWidth);
//
//	// Check we got the expected number of glyphs
//	ASSERT_GT(positionedGlyphs.size(), 0) << "Expected at least one glyph";
//
//	// First line is "A0000000000000000000"
//	EXPECT_EQ(positionedGlyphs[0].codepoint, 'A');
//	EXPECT_EQ(positionedGlyphs[0].position.x, startPos.x);
//	EXPECT_EQ(positionedGlyphs[0].position.y, startPos.y + (16 * 0));
//
//	// Second line is "B1111111111111111111"
//	EXPECT_EQ(positionedGlyphs[20].codepoint, 'B');
//	EXPECT_EQ(positionedGlyphs[20].position.x, startPos.x);
//	EXPECT_EQ(positionedGlyphs[20].position.y, startPos.y + (16 * 1));
//
//	// Third line is "C2222222222222222222"
//	EXPECT_EQ(positionedGlyphs[40].codepoint, 'C');
//	EXPECT_EQ(positionedGlyphs[40].position.x, startPos.x);
//	EXPECT_EQ(positionedGlyphs[40].position.y, startPos.y + (16 * 2));
//
//	// Fourth line is "D3333333333333333333"
//	EXPECT_EQ(positionedGlyphs[60].codepoint, 'D');
//	EXPECT_EQ(positionedGlyphs[60].position.x, startPos.x);
//	EXPECT_EQ(positionedGlyphs[60].position.y, startPos.y + (16 * 3));
//}
//
//TEST_F(GraphicsTextTest, TestMultiNewLine)
//{
//	// The text to shape
//	std::string text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n\n"
//					   "Sed do eiusmod tempor incididunt\nut labore et dolore magna aliqua.";
//
//	// Starting position (for example, top-left corner at (0,0))
//	glm::ivec2 startPos(0, 0);
//
//	// Maximum width for wrapping
//	int maxWidth = 200;
//
//	// Call your Font::wrappedText() method
//	std::vector<PositionedGlyph> positionedGlyphs = _font->wrappedText(text, startPos, maxWidth);
//
//	// Check we got the expected number of glyphs
//	ASSERT_GT(positionedGlyphs.size(), 0) << "Expected at least one glyph";
//
//	// First line is "Lorem ipsum dolor sit "
//	EXPECT_EQ(positionedGlyphs[0].codepoint, 'L');
//	EXPECT_EQ(positionedGlyphs[0].position.x, startPos.x);
//	EXPECT_EQ(positionedGlyphs[0].position.y, startPos.y + (16 * 0));
//
//	// Second line is "amet, consectetur "
//	EXPECT_EQ(positionedGlyphs[22].codepoint, 'a');
//	EXPECT_EQ(positionedGlyphs[22].position.x, startPos.x);
//	EXPECT_EQ(positionedGlyphs[22].position.y, startPos.y + (16 * 1));
//
//	// Third line is "adipiscing elit.\n\n"
//	EXPECT_EQ(positionedGlyphs[40].codepoint, 'a');
//	EXPECT_EQ(positionedGlyphs[40].position.x, startPos.x);
//	EXPECT_EQ(positionedGlyphs[40].position.y, startPos.y + (16 * 2));
//
//	// Skip fourth line
//
//	// Forth line is "Sed do eiusmod tempor "
//	EXPECT_EQ(positionedGlyphs[58].codepoint, 'S');
//	EXPECT_EQ(positionedGlyphs[58].position.x, startPos.x);
//	EXPECT_EQ(positionedGlyphs[58].position.y, startPos.y + (16 * 4));
//
//	// Fifth line is "incididunt\n"
//	EXPECT_EQ(positionedGlyphs[80].codepoint, 'i');
//	EXPECT_EQ(positionedGlyphs[80].position.x, startPos.x);
//	EXPECT_EQ(positionedGlyphs[80].position.y, startPos.y + (16 * 5));
//
//	// Seventh line is "ut labore et dolore "
//	EXPECT_EQ(positionedGlyphs[91].codepoint, 'u');
//	EXPECT_EQ(positionedGlyphs[91].position.x, startPos.x);
//	EXPECT_EQ(positionedGlyphs[91].position.y, startPos.y + (16 * 6));
//
//	// Eighth line is "magna aliqua."
//	EXPECT_EQ(positionedGlyphs[111].codepoint, 'm');
//	EXPECT_EQ(positionedGlyphs[111].position.x, startPos.x);
//	EXPECT_EQ(positionedGlyphs[111].position.y, startPos.y + (16 * 7));
//}
