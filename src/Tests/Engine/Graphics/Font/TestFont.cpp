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

#include "../../../../Engine/Engine.h"
#include "../../../../Engine/Graphics/Font/Font.h"
#include "../../../../Engine/Graphics/Font/FontManager.h"
#include "../../../../Engine/Resource/ResourceSystem.h"
#include "../../../../Engine/Resource/FileProcessor/ImageFile.h"
#include "../../../../Engine/Resource/FileProcessor/ImageBMPFileProcessor.h"

#include <memory>
#include <array>

#include <hb.h>

#include <linebreak.h>

using namespace OpenXcom;

TEST(HarfBuzzTest, InitializeHarfBuzz)
{
	hb_buffer_t* buffer = hb_buffer_create();
	ASSERT_NE(buffer, nullptr); // Ensure buffer creation was successful
	hb_buffer_destroy(buffer);
}

TEST(HarfBuzzTest, ShapeSimpleText)
{
	hb_buffer_t* buffer = hb_buffer_create();
	hb_buffer_add_utf8(buffer, "Hello", -1, 0, -1);
	hb_buffer_guess_segment_properties(buffer);

	hb_font_t* font = hb_font_create(hb_face_create_for_tables(nullptr, nullptr, nullptr));
	hb_shape(font, buffer, nullptr, 0);

	unsigned int glyph_count;
	hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);

	ASSERT_GT(glyph_count, 0); // Ensure we got at least one glyph
	hb_buffer_destroy(buffer);
	hb_font_destroy(font);
}


TEST(HarfBuzzTest, ShapeArabicText)
{
	hb_buffer_t* buffer = hb_buffer_create();
	hb_buffer_add_utf8(buffer, "مرحبا", -1, 0, -1);
	hb_buffer_guess_segment_properties(buffer);

	hb_font_t* font = hb_font_create(hb_face_create_for_tables(nullptr, nullptr, nullptr));
	hb_shape(font, buffer, nullptr, 0);

	unsigned int glyph_count;
	hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);

	ASSERT_GT(glyph_count, 0); // Ensure Arabic text is shaped properly
	hb_buffer_destroy(buffer);
	hb_font_destroy(font);
}

// A sample test using "Lorem ipsum..." text with a newline embedded.
TEST(LibUniWrapperTest, BasicLineBreakTest)
{
	// Test string: contains spaces and a newline.
	std::string text = "Lorem ipsum dolor sit amet,\nconsectetur adipiscing elit.";

	// Get the line break properties for each byte/character.
	std::vector<char> breakProps(text.size());

	// Call the line breaking function with the UTF-8 text.
	set_linebreaks_utf8(reinterpret_cast<const utf8_t*>(text.c_str()), text.size(), "", breakProps.data());

	// Verify that the size of the break properties matches the string length.
	ASSERT_EQ(breakProps.size(), text.size()) << "Expected break property for each character.";

	// Find the index of the newline character.
	size_t newlinePos = text.find('\n');
	ASSERT_NE(newlinePos, std::string::npos) << "Newline not found in test string.";

	// Expect that the newline character is flagged as a MUST break.
	EXPECT_EQ(breakProps[newlinePos], LINEBREAK_MUSTBREAK) << "Newline should be marked as MustBreak.";

	// Optionally, check that a space character is marked as an allowed break.
	// In this example, the first space occurs at index 5 ("Lorem ").
	EXPECT_EQ(breakProps[5], LINEBREAK_ALLOWBREAK) << "Expected space at index 5 to be marked as AllowBreak.";
}

// wrap in an anonymous namespace to avoid name conflicts
namespace
{
#include "../../../../Engine/Graphics/Font/DosFont.h"
}



class FontTest : public TestEngineSuite
{
protected:

	OwningHandle<Palette> _paletteHandle;
	std::unique_ptr<Font> _font;

	void SetUp() override
	{
		_paletteHandle = create16ColorPalette();

		ResourceSystem& resourceSystem = _engine->getEngineContext().getResourceSystem();

		// Load the DOS font
		ImageFile fontTextureFile;
		ImageLoadParams params;

		ASSERT_TRUE(resourceSystem.getImageBMPFileProcessor().load(fontTextureFile, "DosFont", dosFont, DOSFONT_SIZE, params)) << "Failed to load DOS font bitmap";

		OwningHandle<HostImage> hostFontTexture = fontTextureFile.takeImage();
		ASSERT_TRUE(hostFontTexture.isValid()) << "Failed to load DOS font";

		// Transfer the host image to device so we can use it as a font texture
		OwningHandle<DeviceImage> deviceFontTexture = resourceSystem.getImageManager().createDeviceImage(*hostFontTexture);

		// Create the font object
		OpenXcom::FontSettings settings;
		settings.width = 9;
		settings.height = 16;
		settings.spacing = 0;
		settings.defaultPaletteIndex = 1;
		settings.numPaletteEntries = 1;

		_font = std::make_unique<Font>("dosFont", settings, std::move(deviceFontTexture), getAsciiGlyphs(deviceFontTexture.getHandle()));
		_font->setLineSpacing(0);
	}

	void TearDown() override
	{
		_paletteHandle.release();
		_font.reset();
	}
};

TEST_F(FontTest, TestSomeAsciiCharacters)
{
	// Test some ASCII characters
	const Glyph* glyph = _font->getGlyph('A');
	ASSERT_NE(glyph, nullptr);
	EXPECT_EQ(glyph->x, 9);
	EXPECT_EQ(glyph->y, 16);
	EXPECT_EQ(glyph->width, 9);
	EXPECT_EQ(glyph->height, 16);
	EXPECT_EQ(glyph->xOffset, 0);
	EXPECT_EQ(glyph->yOffset, 0);
	EXPECT_EQ(glyph->xAdvance, 9);
	glyph = _font->getGlyph('B');
	ASSERT_NE(glyph, nullptr);
	EXPECT_EQ(glyph->x, 18);
	EXPECT_EQ(glyph->y, 16);
	EXPECT_EQ(glyph->width, 9);
	EXPECT_EQ(glyph->height, 16);
	EXPECT_EQ(glyph->xOffset, 0);
	EXPECT_EQ(glyph->yOffset, 0);
	EXPECT_EQ(glyph->xAdvance, 9);
}

TEST_F(FontTest, TestFontGetExtents)
{
	// The text to shape
	std::string text = "Hello World!";

	// Starting position (for example, top-left corner at (0,0))
	glm::ivec2 startPos(0, 0);

	glm::ivec2 extents = _font->getTextExtents(text);
	EXPECT_EQ(extents.x, 108) << "Expected text width of 99 pixels";
	EXPECT_EQ(extents.y, 16) << "Expected text height of 16 pixels";
}
