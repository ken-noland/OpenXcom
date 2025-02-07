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
#include <cmath>
#include <algorithm>
#include <numbers>

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
		const uint32_t width = 256;
		const uint32_t height = 256;

		// Create a host image with 1 byte per pixel (indexed color).
		ImageManager& imageManager = resourceSystem.getImageManager();
		OwningHandle<HostImage> imageHandle = imageManager.createHostImage("squareColorWheel", glm::ivec2(width, height), ImageFormat::R8);

		// Map the pixel buffer.
		uint8_t* pixels = static_cast<uint8_t*>(imageHandle->map());

		// Adjustable parameters:
		unsigned cellSize = 16; // Size (in pixels) of each cell.

		// Define center. For a square image, use half the width.
		float centerX = width / 2.0f;
		float centerY = height / 2.0f;
		// For a square, using Chebyshev distance the maximum “radius” is half the width.
		float maxRadius = centerX; // i.e. 128 for a 256x256 image.

		// Define discrete levels for quantization.
		const int H_levels = 17; // Number of hue steps.
		const int S_levels = 15; // Number of saturation steps.
		// Total colors for the wheel: 15 * 17 = 255 (using palette indices 1 to 255).

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
					// Compute offset from center.
					float dx = x - centerX;
					float dy = y - centerY;

					// Use Chebyshev distance to stretch the circle into a square.
					float distance = std::max(std::abs(dx), std::abs(dy));

					// Normalize the "distance" for saturation (0 at center, 1 at edge).
					float saturation = distance / maxRadius;
					saturation = std::min(1.0f, saturation); // Clamp to [0,1].

					// Compute angle (hue) in radians.
					float angle = std::atan2(dy, dx);
					// Normalize angle from [-π, π] to [0,1).
					float hue = (angle + static_cast<float>(std::numbers::pi)) / (2.0f * static_cast<float>(std::numbers::pi));

					// Quantize saturation and hue to our discrete grid.
					int s_index = static_cast<int>(saturation * (S_levels - 1) + 0.5f); // Range: 0 .. S_levels-1.
					int h_index = static_cast<int>(hue * (H_levels - 1) + 0.5f);        // Range: 0 .. H_levels-1.

					// For a near–white center (saturation 0), force hue to 0 so that all white pixels match.
					if (s_index == 0)
						h_index = 0;

					// Compute the palette index (reserve index 0 for black).
					int paletteIndex = 1 + s_index * H_levels + h_index;
					pixels[y * width + x] = static_cast<uint8_t>(paletteIndex);
				}
			}
		}
		imageHandle->unmap();

		// --- Build the palette ---
		// There are 256 entries in the palette. Entry 0 is black.
		PackedColor paletteColors[256];

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
				Color c = hsv_to_rgb(hue, sat, 1.0f);
				int paletteIndex = 1 + s * H_levels + h; // Runs from 1 to 255.
				paletteColors[paletteIndex].set(c.r, c.g, c.b, 255);
			}
		}

		PaletteManager& paletteManager = resourceSystem.getPaletteManager();
		OwningHandle<Palette> paletteHandle = paletteManager.createPalette("squareColorWheelPalette", paletteColors, 256);

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
