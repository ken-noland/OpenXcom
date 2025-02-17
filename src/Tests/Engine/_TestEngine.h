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

#include "../../Engine/Engine.h"
#include "../../Engine/Graphics/GraphicsSurface.h"
#include "../../Engine/Graphics/Common/GameSurface.h"
#include "../../Engine/Graphics/Common/WindowSurface.h"
#include "../../Engine/Graphics/Image/Image.h"
#include "../../Engine/Graphics/Image/ImageManager.h"
#include "../../Engine/Graphics/Palette/Palette.h"
#include "../../Engine/Graphics/Palette/PaletteManager.h"
#include "../../Engine/Graphics/Font/Font.h"
#include "../../Engine/Graphics/Font/FontManager.h"
#include "../../Engine/Resource/Handle.h"
#include "../../Engine/Resource/ResourceSystem.h"
#include "../../Engine/Resource/FileProcessor/ImageFile.h"
#include "../../Engine/Resource/FileProcessor/ImageBMPFileProcessor.h"

#include <numbers>
#include <filesystem>
#include <algorithm>

#include <lodepng.h>

// wrap in an anonymous namespace to avoid name conflicts
namespace
{
#include "../../Engine/Graphics/Font/DosFont.h"
}


class TestEngineSuite : public ::testing::Test
{
protected:
	bool FORCE_REGENERATE_BASELINE = false;

	static std::unique_ptr<OpenXcom::Engine> _engine;

	static std::filesystem::path _dataPath;
	static std::filesystem::path _configPath;
	static std::filesystem::path _userPath;

	static std::unique_ptr<OpenXcom::GameSurface> _gameSurface;
	static std::unique_ptr<OpenXcom::WindowSurface> _windowSurface;

public:

	static void SetUpTestSuite()
	{
		ASSERT_EQ(_engine, nullptr) << "Engine already initialized.";
		ASSERT_EQ(_gameSurface, nullptr) << "GameSurface already initialized.";
		ASSERT_EQ(_windowSurface, nullptr) << "WindowSurface already initialized.";

		std::filesystem::path path = TEST_DATA_DIR;
		_dataPath = path / "Data";
		_configPath = path / "Config";
		_userPath = path / "User";

		std::vector<std::string> args = {"-data", _dataPath.string(), "-config", _configPath.string(), "-user", _userPath.string(), "-headless"};
		_engine = std::make_unique<OpenXcom::Engine>(args);
		ASSERT_NE(_engine, nullptr) << "Failed to create Engine.";

		_gameSurface = std::make_unique<OpenXcom::GameSurface>(_engine->getEngineContext());
		ASSERT_NE(_gameSurface, nullptr) << "Failed to create GameSurface.";

		_windowSurface = std::make_unique<OpenXcom::WindowSurface>(_engine->getEngineContext(), *_gameSurface);
		ASSERT_NE(_windowSurface, nullptr) << "Failed to create WindowSurface.";
	}

	static void TearDownTestSuite()
	{
		_gameSurface.reset();
		_windowSurface.reset();
		_engine.reset();
	}

	OpenXcom::OwningHandle<OpenXcom::Palette> create16ColorPalette()
	{
		// set up the palette
		OpenXcom::PackedColor paletteData[] = {
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

		OpenXcom::PaletteManager& paletteManager = _engine->getEngineContext().getResourceSystem().getPaletteManager();
		return paletteManager.createPalette("16colors", paletteData, 16);
	}

	OpenXcom::OwningHandle<OpenXcom::Palette> createAnsiColorPalette()
	{
		// set up the palette
		OpenXcom::PackedColor paletteData[] = {
			0x000000FF, // 0: Black       (0,0,0)
			0xAA0000FF, // 1: Red         (170,0,0)
			0x00AA00FF, // 2: Green       (0,170,0)
			0xAA5500FF, // 3: Yellow      (170,85,0)
			0x0000AAFF, // 4: Blue        (0,0,170)
			0xAA00AAFF, // 5: Magenta     (170,0,170)
			0x00AAAAFF, // 6: Cyan        (0,170,170)
			0xAAAAAAFF, // 7: White       (170,170,170)
			0x555555FF, // 8: Bright Black (Gray) (85,85,85)
			0xFF5555FF, // 9: Bright Red  (255,85,85)
			0x55FF55FF, // 10: Bright Green (85,255,85)
			0xFFFF55FF, // 11: Bright Yellow (255,255,85)
			0x5555FFFF, // 12: Bright Blue (85,85,255)
			0xFF55FFFF, // 13: Bright Magenta (255,85,255)
			0x55FFFFFF, // 14: Bright Cyan (85,255,255)
			0xFFFFFFFF  // 15: Bright White (255,255,255)
		};

		OpenXcom::PaletteManager& paletteManager = _engine->getEngineContext().getResourceSystem().getPaletteManager();
		return paletteManager.createPalette("16colors", paletteData, 16);
	}

	// Helper function to interpolate between two colors.
	OpenXcom::PackedColor lerpColor(OpenXcom::PackedColor start, OpenXcom::PackedColor end, float t)
	{
		// Interpolate each channel
		uint8_t r = static_cast<uint8_t>(start.r() + t * (end.r() - start.r()));
		uint8_t g = static_cast<uint8_t>(start.g() + t * (end.g() - start.g()));
		uint8_t b = static_cast<uint8_t>(start.b() + t * (end.b() - start.b()));
		uint8_t a = static_cast<uint8_t>(start.a() + t * (end.a() - start.a()));

		// Pack channels back into a color.
		return (r << 24) | (g << 16) | (b << 8) | a;
	}

	OpenXcom::OwningHandle<OpenXcom::Palette> createAnsiColorPalette81()
	{
		// set up the palette targets(targets for the lerp function)
		OpenXcom::PackedColor paletteTargets[] = {
			0x000000FF, // 0: Black       (0,0,0)
			0xAA0000FF, // 1: Red         (170,0,0)
			0x00AA00FF, // 2: Green       (0,170,0)
			0xAA5500FF, // 3: Yellow      (170,85,0)
			0x0000AAFF, // 4: Blue        (0,0,170)
			0xAA00AAFF, // 5: Magenta     (170,0,170)
			0x00AAAAFF, // 6: Cyan        (0,170,170)
			0xAAAAAAFF, // 7: White       (170,170,170)
			0x555555FF, // 8: Bright Black (Gray) (85,85,85)
			0xFF5555FF, // 9: Bright Red  (255,85,85)
			0x55FF55FF, // 10: Bright Green (85,255,85)
			0xFFFF55FF, // 11: Bright Yellow (255,255,85)
			0x5555FFFF, // 12: Bright Blue (85,85,255)
			0xFF55FFFF, // 13: Bright Magenta (255,85,255)
			0x55FFFFFF, // 14: Bright Cyan (85,255,255)
			0xFFFFFFFF  // 15: Bright White (255,255,255)
		};

		// set up the palette
		OpenXcom::PackedColor paletteData[81];
		paletteData[0] = 0x000000FF;
		for (int i = 0; i < 15; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				float t = (5.f-j) / 4.0f;
				paletteData[1 + ((i * 5) + j)] = lerpColor(paletteTargets[0], paletteTargets[i + 1], t);
			}
		}

		
		OpenXcom::PaletteManager& paletteManager = _engine->getEngineContext().getResourceSystem().getPaletteManager();
		return paletteManager.createPalette("81colors", paletteData, 81);
	}


	/**
	 * Captures the game surface into a HostImage.
	 */
	OpenXcom::OwningHandle<OpenXcom::HostImage> captureGameSurface()
	{
		_windowSurface->update(); // render the surface once

		OpenXcom::OwningHandle<OpenXcom::HostImage> hostImageHandle = _engine->getResourceSystem().getImageManager().createHostImage("screenCapture", _gameSurface->getScreenSize(), OpenXcom::ImageFormat::R8G8B8A8);
		if (!hostImageHandle.isValid())
		{
			ADD_FAILURE() << "Failed to create HostImage.";
			return OpenXcom::OwningHandle<OpenXcom::HostImage>();
		}

		_gameSurface->captureFrame(hostImageHandle.get());
		return hostImageHandle;
	}

	/**
	 * Compares a captured image with a baseline.
	 */
	void compareWithBaseline(const OpenXcom::OwningHandle<OpenXcom::HostImage>& hostImageHandle, const std::filesystem::path& baselinePath)
	{
		OpenXcom::HostImage& hostImage = hostImageHandle.get();
		const uint8_t* pixels = static_cast<const uint8_t*>(hostImage.map());

		// Generate a baseline if it doesn't exist
		if (!std::filesystem::exists(baselinePath) || FORCE_REGENERATE_BASELINE)
		{
			unsigned error = lodepng::encode(baselinePath.string().c_str(), pixels, hostImage.getWidth(), hostImage.getHeight(), LodePNGColorType::LCT_RGBA, 8);
			EXPECT_EQ(error, 0) << "Failed to save baseline image.";
		}
		else
		{
			// Load the baseline image
			std::vector<unsigned char> baseline;
			uint32_t width = 0, height = 0;

			unsigned error = lodepng::decode(baseline, width, height, baselinePath.string().c_str());
			ASSERT_EQ(error, 0) << "Failed to load baseline image.";

			// Compare dimensions
			ASSERT_EQ(width, hostImage.getExtent().x) << "Image width mismatch.";
			ASSERT_EQ(height, hostImage.getExtent().y) << "Image height mismatch.";

			// Compare pixel data
			ASSERT_EQ(baseline.size(), hostImage.getExtent().x * hostImage.getExtent().y * 4) << "Image data size mismatch.";
			for (size_t i = 0; i < baseline.size(); ++i)
			{
				std::size_t x = (i / 4) % hostImage.getExtent().x;
				std::size_t y = (i / 4) / hostImage.getExtent().x;
				ASSERT_EQ(baseline[i], pixels[i]) << "Pixel mismatch at index " << i << "(x=" << x << " y=" << y << ")";
			}
		}

		hostImage.unmap();
	}

		// Convert an HSV color (with h in [0,1], s in [0,1], and v in [0,1])
	// to an RGB color.
	OpenXcom::PackedColor hsv_to_rgb(float h, float s, float v)
	{
		float r = 0.f, g = 0.f, b = 0.f;
		int i = int(h * 6);
		float f = h * 6 - i;
		float p = v * (1 - s);
		float q = v * (1 - f * s);
		float t = v * (1 - (1 - f) * s);

		switch (i % 6)
		{
		case 0:
			r = v, g = t, b = p;
			break;
		case 1:
			r = q, g = v, b = p;
			break;
		case 2:
			r = p, g = v, b = t;
			break;
		case 3:
			r = p, g = q, b = v;
			break;
		case 4:
			r = t, g = p, b = v;
			break;
		case 5:
			r = v, g = p, b = q;
			break;
		}
		return OpenXcom::PackedColor{static_cast<unsigned char>(r * 255),
									 static_cast<unsigned char>(g * 255),
									 static_cast<unsigned char>(b * 255)};
	}

	// Create an ImageFile
	OpenXcom::ImageFile createImage()
	{
		OpenXcom::ResourceSystem& resourceSystem = _engine->getEngineContext().getResourceSystem();
		const uint32_t width = 256;
		const uint32_t height = 256;

		// Create a host image with 1 byte per pixel (indexed color).
		OpenXcom::ImageManager& imageManager = resourceSystem.getImageManager();
		OpenXcom::OwningHandle<OpenXcom::HostImage> imageHandle = imageManager.createHostImage("squareColorWheel", glm::ivec2(width, height), OpenXcom::ImageFormat::R8);

		// Map the pixel buffer.
		uint8_t* pixels = static_cast<uint8_t*>(imageHandle->map());

		// Adjustable parameters:
		unsigned cellSize = 16;        // Size (in pixels) of each cell.
		const int borderThreshold = 1; // Number of pixels near the grid line to treat as “border”.

		// Define center. For a square image, use half the width.
		float centerX = width / 2.0f;
		float centerY = height / 2.0f;
		// Using Chebyshev distance, the maximum “radius” is half the width.
		float maxRadius = centerX; // i.e. 128 for a 256x256 image.

		// Define discrete levels for quantization.
		const int H_levels = 17; // Number of hue steps.
		const int S_levels = 15; // Number of saturation steps.
		// (Thus the interior colors are stored in palette indices 1 to 255.)

		for (unsigned y = 0; y < height; y++)
		{
			for (unsigned x = 0; x < width; x++)
			{
				// Compute offset from center.
				float dx = x - centerX;
				float dy = y - centerY;
				// Use Chebyshev distance to stretch the circle into a square.
				float distance = std::max(std::abs(dx), std::abs(dy));
				// Normalize for saturation.
				float saturation = std::min(distance / maxRadius, 1.0f);

				// Compute the polar angle (hue) from the image center.
				float angle = std::atan2(dy, dx);
				float hue = (angle + static_cast<float>(std::numbers::pi)) / (2.0f * static_cast<float>(std::numbers::pi));

				// Determine local coordinates within the current cell.
				int localX = x % cellSize;
				int localY = y % cellSize;
				// Compute how far from the nearest grid line we are.
				int d1 = std::min(localX, localY);
				int d2 = std::min(cellSize - 1 - localX, cellSize - 1 - localY);
				int distToBorder = std::min(d1, d2);

				// If we're in the border zone, use a modified color.
				if (distToBorder < borderThreshold)
				{
					// Compute a simple fade factor: 1 at the grid line, 0 at borderThreshold pixels in.
					float fade = 1.0f - (distToBorder / static_cast<float>(borderThreshold));
					// Here we “simulate” a fade by choosing either black or the inverted color.
					// (Because our palette only covers full brightness, we choose a cutoff value.)
					if (fade < 0.5f)
					{
						// Fade is low enough: use black.
						pixels[y * width + x] = 0;
					}
					else
					{
						// Compute the inverted hue (complementary color).
						float invHue = std::fmod(hue + 0.5f, 1.0f);
						// Quantize the inverted hue and the original saturation.
						int s_index = static_cast<int>(saturation * (S_levels - 1) + 0.5f);
						int h_index = static_cast<int>(invHue * (H_levels - 1) + 0.5f);
						// For a near–white center (saturation 0), force hue to 0.
						if (s_index == 0)
							h_index = 0;
						int paletteIndex = 1 + s_index * H_levels + h_index;
						pixels[y * width + x] = static_cast<uint8_t>(paletteIndex);
					}
				}
				else
				{
					// Regular cell interior: use the computed hue and saturation.
					int s_index = static_cast<int>(saturation * (S_levels - 1) + 0.5f);
					int h_index = static_cast<int>(hue * (H_levels - 1) + 0.5f);
					if (s_index == 0)
						h_index = 0;
					int paletteIndex = 1 + s_index * H_levels + h_index;
					pixels[y * width + x] = static_cast<uint8_t>(paletteIndex);
				}
			}
		}
		imageHandle->unmap();

		// --- Build the palette ---
		// There are 256 entries in the palette. Entry 0 is black.
		OpenXcom::PackedColor paletteColors[256];

		// Palette entry 0: black.
		paletteColors[0].set(0, 0, 0, 255);

		// Fill in indices 1 to 255 using the same quantization.
		for (int s = 0; s < S_levels; s++)
		{
			// Normalize the saturation level.
			float sat = s / static_cast<float>(S_levels - 1);
			for (int h = 0; h < H_levels; h++)
			{
				// For s == 0, force hue to 0 to always get white.
				float hue = (s == 0) ? 0.0f : h / static_cast<float>(H_levels - 1);
				OpenXcom::PackedColor c = hsv_to_rgb(hue, sat, 1.0f);
				int paletteIndex = 1 + s * H_levels + h; // Runs from 1 to 255.
				paletteColors[paletteIndex] = c;
			}
		}

		OpenXcom::PaletteManager& paletteManager = resourceSystem.getPaletteManager();
		OpenXcom::OwningHandle<OpenXcom::Palette> paletteHandle = paletteManager.createPalette("squareColorWheelPalette", paletteColors, 256);

		return OpenXcom::ImageFile(std::move(imageHandle), std::move(paletteHandle));
	}

	// helper function to get the glyph from the font
	std::array<OpenXcom::Glyph, 128> getAsciiGlyphs(OpenXcom::ResourceHandle<OpenXcom::DeviceImage> image)
	{
		std::array<OpenXcom::Glyph, 128> asciiGlyphs;
		memset(asciiGlyphs.data(), 0, asciiGlyphs.size() * sizeof(OpenXcom::Glyph));

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
				static_cast<int8_t>(charWidth), // Fixed width spacing
				image
			};
		}

		return asciiGlyphs;
	}

	OpenXcom::OwningHandle<OpenXcom::Font> createDosFont()
	{
		OpenXcom::ResourceSystem& resourceSystem = _engine->getEngineContext().getResourceSystem();

		// Load the DOS font
		OpenXcom::ImageFile fontTextureFile;
		OpenXcom::ImageLoadParams params;

		resourceSystem.getImageBMPFileProcessor().load(fontTextureFile, "DosFont", dosFont, DOSFONT_SIZE, params);
		OpenXcom::OwningHandle<OpenXcom::HostImage> hostFontTexture = fontTextureFile.takeImage();

		// Transfer the host image to device so we can use it as a font texture
		OpenXcom::OwningHandle<OpenXcom::DeviceImage> deviceFontTexture = resourceSystem.getImageManager().createDeviceImage(*hostFontTexture);
		OpenXcom::ResourceHandle<OpenXcom::DeviceImage> fontTextureHandle = deviceFontTexture.getHandle();
		std::vector<OpenXcom::OwningHandle<OpenXcom::DeviceImage>> textures;
		textures.push_back(std::move(deviceFontTexture));

		// Create the font object
		OpenXcom::FontSettings settings;
		settings.width = 9;
		settings.height = 16;
		settings.spacing = 0;
		settings.defaultPaletteIndex = 1;
		settings.numPaletteEntries = 1;

		OpenXcom::OwningHandle<OpenXcom::Font> font = resourceSystem.getFontManager().load("dosFont", settings, std::move(textures), getAsciiGlyphs(fontTextureHandle));
		font->setLineSpacing(0);

		return font;
	}

};
