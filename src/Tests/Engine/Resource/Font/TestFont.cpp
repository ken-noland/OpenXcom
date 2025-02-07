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
#include <gtest/gtest.h>

#include "../../../../Engine/Engine.h"
#include "../../../../Engine/Resource/ResourceSystem.h"
#include "../../../../Engine/Resource/Font/Font.h"
#include "../../../../Engine/Resource/FileProcessor/ImageFile.h"
#include "../../../../Engine/Resource/FileProcessor/ImageFileProcessor.h"
#include "../../../../Engine/Resource/FileProcessor/ImageBMPFileProcessor.h"
#include "../../../../Engine/Resource/FileProcessor/ImagePNGFileProcessor.h"
#include "../../../../Engine/Graphics/Buffer/Buffer.h"
#include "../../../../Engine/Graphics/Buffer/BufferManager.h"
#include "../../../../Engine/Graphics/Image/Image.h"
#include "../../../../Engine/Graphics/Image/ImageManager.h"
#include "../../../../Engine/Graphics/Palette/Palette.h"
#include "../../../../Engine/Graphics/Palette/PaletteManager.h"

#include <memory>
#include <filesystem>
#include <cmath>
#include <algorithm>
#include <numbers>
#include <unordered_map>
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
#include "../../../../Engine/Resource/Font/DosFont.h"
}


// helper function to get the glyph from the font
std::array<Glyph, 128> getAsciiGlyphs()
{
	std::array<Glyph, 128> asciiGlyphs;
	std::memset(asciiGlyphs.data(), 0, asciiGlyphs.size() * sizeof(Glyph));

	int charWidth = 9;
	int charHeight = 16;
	int textureWidth = 288;
	int textureHeight = 48;
	int charsPerRow = textureWidth / charWidth;

	std::string characters =
		" !\"#$%&'()*+,-./0123456789:;<=>?"
		"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^"
		"_`abcdefghijklmnopqrstuvwxyz{|}~";

	for (size_t i = 0; i < characters.size(); i++)
	{
		int x = static_cast<int>((i % charsPerRow) * charWidth);
		int y = static_cast<int>((i / charsPerRow) * charHeight);

		assert(characters[i] < 127);

		asciiGlyphs[characters[i]] = {
			static_cast<uint16_t>(x),
			static_cast<uint16_t>(y),
			static_cast<uint16_t>(charWidth),
			static_cast<uint16_t>(charHeight),
			0, 0,
			static_cast<int8_t>(charWidth) // Fixed width spacing
		};
	}

	return asciiGlyphs;
}

class FontTest : public ::testing::Test
{
protected:
	static std::unique_ptr<Engine> _engine;

	static std::filesystem::path _dataPath;
	static std::filesystem::path _configPath;
	static std::filesystem::path _userPath;

	static PaletteManager::OwningHandle _paletteHandle;

	std::unique_ptr<Font> _font;

	static void SetUpTestSuite()
	{
		std::filesystem::path path = TEST_DATA_DIR;
		_dataPath = path / "Data";
		_configPath = path / "Config";
		_userPath = path / "User";

		std::vector<std::string> args = {"-data", _dataPath.string(), "-config", _configPath.string(), "-user", _userPath.string(), "-headless"};
		_engine = std::make_unique<Engine>(args);

		// Set up a palette, if your BMP relies on a specific palette (indexed mode)
		PackedColor paletteData[] = {
			0x000000FF, // 0 - Black
			0xFFFFFFFF, // 1 - White
			0x808080FF, // 2 - Gray
			0xFF0000FF, // 3 - Red
			0x00FF00FF, // 4 - Green
			0x0000FFFF, // 5 - Blue
			0xFFFF00FF, // 6 - Yellow
			0xFF00FFFF, // 7 - Magenta
			0x00FFFFFF, // 8 - Cyan
			0xFFA500FF, // 9 - Orange
			0x8A2BE2FF, // 10 - Blue Violet
			0x008080FF, // 11 - Teal
			0x4B0082FF, // 12 - Indigo
			0x800000FF, // 13 - Maroon
			0x808000FF, // 14 - Olive
			0x8B4513FF  // 15 - Saddle Brown
		};

		PaletteManager& paletteManager = _engine->getEngineContext().getResourceSystem().getPaletteManager();
		_paletteHandle = paletteManager.createPalette("16colors", paletteData, 16);
	}

	static void TearDownTestSuite()
	{
		_paletteHandle.release();
		_engine.reset();
	}

	void SetUp() override
	{
		ResourceSystem& resourceSystem = _engine->getEngineContext().getResourceSystem();

		// Load the DOS font
		ImageFile fontTextureFile = resourceSystem.getImageBMPFileProcessor().load("DosFont", dosFont, DOSFONT_SIZE, false);
		OwningHandle<HostImage> hostFontTexture = fontTextureFile.takeImage();
		ASSERT_TRUE(hostFontTexture.isValid()) << "Failed to load DOS font";

		// Transfer the host image to device so we can use it as a font texture
		OwningHandle<DeviceImage> deviceFontTexture = resourceSystem.getImageManager().createDeviceImage(*hostFontTexture);

		// Create the font object
		_font = std::make_unique<Font>(std::move(deviceFontTexture), getAsciiGlyphs());
	}

	void TearDown() override
	{
		_font.reset();
	}
};

std::unique_ptr<Engine> FontTest::_engine(nullptr);

std::filesystem::path FontTest::_dataPath;
std::filesystem::path FontTest::_configPath;
std::filesystem::path FontTest::_userPath;

PaletteManager::OwningHandle FontTest::_paletteHandle;

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

TEST_F(FontTest, TestHelloWorldShape)
{
	// The text to shape
	std::string text = "Hello World!";

	// Starting position (for example, top-left corner at (0,0))
	glm::ivec2 startPos(0, 0);

	// Call your Font::shapeText() method
	std::vector<PositionedGlyph> positionedGlyphs = _font->shapeText(text, startPos);

	// Check we got the expected number of glyphs
	ASSERT_EQ(positionedGlyphs.size(), text.size()) << "Expected one glyph per character";

	// Now verify each glyph's advance and offsets.
	// We expect each glyph's advance to be 9 (since 9 * 64 = 576 in fixed-point)
	// and offsets to be zero.
	uint32_t expectedAdvance = 0;
	for (size_t i = 0; i < positionedGlyphs.size(); i++)
	{
		const PositionedGlyph& glyph = positionedGlyphs[i];
		EXPECT_EQ(glyph.advance, 9) << "Glyph index " << i << " has incorrect advance";
		EXPECT_EQ(glyph.position.x, startPos.x + expectedAdvance) << "Glyph index " << i << " has incorrect x position";
		EXPECT_EQ(glyph.offsetX, 0) << "Glyph index " << i << " has nonzero x offset";
		EXPECT_EQ(glyph.offsetY, 0) << "Glyph index " << i << " has nonzero y offset";
		expectedAdvance += glyph.advance;
	}
}

TEST_F(FontTest, TestLoremIpsumMultiLine)
{
	// The text to shape
	std::string text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
					   "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";

	// Starting position (for example, top-left corner at (0,0))
	glm::ivec2 startPos(0, 0);

	// Maximum width for wrapping
	int maxWidth = 80;

	// Call your Font::wrappedText() method
	std::vector<PositionedGlyph> positionedGlyphs = _font->wrappedText(text, startPos, maxWidth);

	// Check we got the expected number of glyphs
	ASSERT_GT(positionedGlyphs.size(), 0) << "Expected at least one glyph";
}
