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

using namespace OpenXcom;

class ImageFileProcessorTest : public ::testing::Test
{
protected:
	static std::unique_ptr<Engine> _engine;

	static std::filesystem::path _dataPath;
	static std::filesystem::path _configPath;
	static std::filesystem::path _userPath;

	static void SetUpTestSuite()
	{
		std::filesystem::path path = TEST_DATA_DIR;
		_dataPath = path / "Data";
		_configPath = path / "Config";
		_userPath = path / "User";

		std::vector<std::string> args = {"-data", _dataPath.string(), "-config", _configPath.string(), "-user", _userPath.string(), "-headless"};
		_engine = std::make_unique<Engine>(args);
	}

	static void TearDownTestSuite()
	{
		_engine.reset();
	}

	// Simple structure to hold RGB color values.
	struct Color
	{
		unsigned char r, g, b;
	};

	// Convert an HSV color (with h in [0,1], s in [0,1], and v in [0,1])
	// to an RGB color.
	Color hsv_to_rgb(float h, float s, float v)
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
		return Color{static_cast<unsigned char>(r * 255),
					 static_cast<unsigned char>(g * 255),
					 static_cast<unsigned char>(b * 255)};
	}

	// Create an ImageFile
	ImageFile createImage()
	{
		ResourceSystem& resourceSystem = _engine->getEngineContext().getResourceSystem();

		uint32_t width = 256;
		uint32_t height = 256;

		// create a host image
		ImageManager& imageManager = resourceSystem.getImageManager();
		OwningHandle<HostImage> imageHandle = imageManager.createHostImage("testImage", glm::ivec2(width, height), ImageFormat::R8);		

		// Adjustable parameters:
		unsigned cellSize = 16;        // Size (in pixels) of each cell.
		unsigned subCellSize = 4;      // Spacing (in pixels) for sub-cell lines.
		unsigned subPaletteOffset = 16; // Palette offset for sub-cell lines.

		// Get an address to the pixels to allow us to write to them.
		uint8_t* pixels = static_cast<uint8_t*>(imageHandle->map());

		// Create a 16x16 grid of cells (16 cells per row, 16 rows = 256 cells).
		// The outer grid lines (every cellSize pixels) use palette index 0 (black).
		// Inside each cell, pixels on a sub-grid boundary (every subCellSize pixels)
		// use the cell's base palette index plus subPaletteOffset (wrapped modulo 256),
		// and other pixels use the base cell palette index.
		// Note: cell indices are computed from the cell coordinates, and we add 1
		// so that index 0 remains reserved for the grid.
		for (unsigned y = 0; y < height; y++)
		{
			for (unsigned x = 0; x < width; x++)
			{
				if (x % cellSize == 0 || y % cellSize == 0)
				{
					// Outer grid line.
					pixels[y * width + x] = 0;
				}
				else
				{
					// Determine which cell we are in.
					unsigned cell_x = x / cellSize;
					unsigned cell_y = y / cellSize;
					// Base cell index (using indices 1..255)
					unsigned baseIndex = cell_y * 16 + cell_x + 1;
					// Determine local coordinates within the cell.
					unsigned localX = x % cellSize;
					unsigned localY = y % cellSize;
					// If we're on a sub-grid line inside the cell, use baseIndex plus subPaletteOffset.
					if ((localX % subCellSize == 0) || (localY % subCellSize == 0))
					{
						pixels[y * width + x] = static_cast<unsigned char>((baseIndex + subPaletteOffset) % 256);
					}
					else
					{
						// Regular cell interior.
						pixels[y * width + x] = static_cast<unsigned char>(baseIndex % 256);
					}
				}
			}
		}

		imageHandle->unmap();

		// --- Palette Setup ---
		// Create a palette of 256 colors, each palette entry is 4 bytes: R, G, B, A.
		// We want index 0 to be used for grid lines. Here we choose black.
		PackedColor paletteColors[256];

		// Palette entry for index 0 (grid lines): black.
		paletteColors[0].set(0, 0, 0, 255);

		// Generate unique colors for indices 1 to 255 using an HSV color wheel.
		for (unsigned i = 1; i < 256; i++)
		{
			// Map i-1 from 0 to 254 into the hue range [0, 1).
			float hue = (i - 1) / 255.0f;
			Color c = hsv_to_rgb(hue, 1.0f, 1.0f);
			paletteColors[i].set(c.r, c.g, c.b, 255);
		}

		// create a palette. It's worth noting that the palette is always
		//	constructed on the device, so we will need to transfer it back
		//	to the CPU later for reading.
		PaletteManager& paletteManager = resourceSystem.getPaletteManager();
		OwningHandle<Palette> paletteHandle = paletteManager.createPalette("testPalette", paletteColors, 256);
		
		return ImageFile(std::move(imageHandle), std::move(paletteHandle));
	}


};

std::unique_ptr<Engine> ImageFileProcessorTest::_engine = nullptr;

std::filesystem::path ImageFileProcessorTest::_dataPath;
std::filesystem::path ImageFileProcessorTest::_configPath;
std::filesystem::path ImageFileProcessorTest::_userPath;


TEST_F(ImageFileProcessorTest, TestBmp8bit)
{
	ImageFile generatedImage = createImage();
	ASSERT_TRUE(generatedImage.getImageHandle().isValid()) << "Image handle should be valid";
	ASSERT_NE(&generatedImage.getImage(), nullptr) << "Image should not be null";
	ASSERT_TRUE(generatedImage.getPaletteHandle().isValid()) << "Palette handle should be valid";
	ASSERT_EQ(generatedImage.getImage().getFormat(), ImageFormat::R8) << "Generated image format should be R8";

	ImageBMPFileProcessor& bmpProcessor = _engine->getEngineContext().getResourceSystem().getImageBMPFileProcessor();

	// Save the image
	std::filesystem::path path = _dataPath / "Test" / "BMP" / "001_generated.bmp";
	bmpProcessor.save(path, generatedImage);

	// Load the image back
	ImageFile loadedImage = bmpProcessor.load("loaded_bmp_bitmap", path);
	ASSERT_TRUE(loadedImage.getImageHandle().isValid()) << "Image handle should be valid";
	ASSERT_NE(&loadedImage.getImage(), nullptr) << "Image should not be null";
	ASSERT_FALSE(loadedImage.getPaletteHandle().isValid()) << "Palette handle should not be valid(we didn't request it to load)";

	// Check image format
	ASSERT_EQ(loadedImage.getImage().getFormat(), ImageFormat::R8) << "Image format mismatch";

	// Check the image data
	HostImage& loadedHostImage = loadedImage.getImage();
	const uint8_t* loadedPixels = static_cast<const uint8_t*>(loadedHostImage.map());
	const uint8_t* generatedPixels = static_cast<const uint8_t*>(generatedImage.getImage().map());

	for (uint32_t y = 0; y < loadedHostImage.getHeight(); y++)
	{
		for (uint32_t x = 0; x < loadedHostImage.getWidth(); x++)
		{
			ASSERT_EQ(loadedPixels[y * loadedHostImage.getWidth() + x], generatedPixels[y * loadedHostImage.getWidth() + x]) << "Pixel mismatch at (" << x << ", " << y << ")";
		}
	}

	loadedHostImage.unmap();
	generatedImage.getImage().unmap();
}


TEST_F(ImageFileProcessorTest, TestPng8bit)
{
	ImageFile generatedImage = createImage();
	ASSERT_TRUE(generatedImage.getImageHandle().isValid()) << "Image handle should be valid";
	ASSERT_NE(&generatedImage.getImage(), nullptr) << "Image should not be null";
	ASSERT_TRUE(generatedImage.getPaletteHandle().isValid()) << "Palette handle should be valid";
	ASSERT_EQ(generatedImage.getImage().getFormat(), ImageFormat::R8) << "Generated image format should be R8";

	ImagePNGFileProcessor& pngProcessor = _engine->getEngineContext().getResourceSystem().getImagePNGFileProcessor();

	// Save the image
	std::filesystem::path path = _dataPath / "Test" / "BMP" / "001_generated.png";
	pngProcessor.save(path, generatedImage);

	// Load the image back
	ImageFile loadedImage = pngProcessor.load("loaded_bmp_bitmap", path);
	ASSERT_TRUE(loadedImage.getImageHandle().isValid()) << "Image handle should be valid";
	ASSERT_NE(&loadedImage.getImage(), nullptr) << "Image should not be null";
	ASSERT_FALSE(loadedImage.getPaletteHandle().isValid()) << "Palette handle should not be valid(we didn't request it to load)";

	// Check image format
	ASSERT_EQ(loadedImage.getImage().getFormat(), ImageFormat::R8) << "Image format mismatch";

	// Check the image data
	HostImage& loadedHostImage = loadedImage.getImage();
	const uint8_t* loadedPixels = static_cast<const uint8_t*>(loadedHostImage.map());
	const uint8_t* generatedPixels = static_cast<const uint8_t*>(generatedImage.getImage().map());

	for (uint32_t y = 0; y < loadedHostImage.getHeight(); y++)
	{
		for (uint32_t x = 0; x < loadedHostImage.getWidth(); x++)
		{
			ASSERT_EQ(loadedPixels[y * loadedHostImage.getWidth() + x], generatedPixels[y * loadedHostImage.getWidth() + x]) << "Pixel mismatch at (" << x << ", " << y << ")";
		}
	}

	loadedHostImage.unmap();
	generatedImage.getImage().unmap();
}
