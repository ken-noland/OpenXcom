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

#include "../../../Engine/Engine.h"
#include "../../../Engine/Graphics/Common/GameSurface.h"
#include "../../../Engine/Graphics/Image/Image.h"
#include "../../../Engine/Graphics/Image/ImageManager.h"
#include "../../../Engine/Graphics/Palette/Palette.h"
#include "../../../Engine/Graphics/Palette/PaletteManager.h"
#include "../../../Engine/Graphics/Types/PackedColor.h"

#include "../../../Engine/Resource/ResourceManager.h"
#include "../../../Engine/Resource/ResourceSystem.h"
#include "../../../Engine/Resource/FileProcessor/ImageFile.h"
#include "../../../Engine/Resource/FileProcessor/ImageBMPFileProcessor.h"

#include <filesystem>

extern "C" {
#include <libbmp.h>
}

// wrap in an anonymous namespace to avoid name conflicts
namespace
{
#include "../../../Engine/Graphics/Font/DosFont.h"
}


#include <lodepng.h>
#include <memory>
#include <vector>

namespace
{
bool FORCE_REGENERATE_BASELINE = false;
}

using namespace OpenXcom;

class BMPTest : public ::testing::Test
{
protected:
	std::unique_ptr<Engine> _engine;
	std::filesystem::path _dataPath;
	std::filesystem::path _configPath;
	std::filesystem::path _userPath;

	OwningHandle<Palette> _paletteHandle;

	void SetUp() override
	{
		std::filesystem::path path = TEST_DATA_DIR;
		_dataPath = path / "data";
		_configPath = path / "config";
		_userPath = path / "user";

		std::vector<std::string> args = {"-data", _dataPath.string(),
										 "-config", _configPath.string(),
										 "-user", _userPath.string(),
										 "-headless"};
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

	void TearDown() override
	{
		_paletteHandle.reset();
		_engine.reset();
	}

	/**
	 * Compares a loaded HostImage with a baseline image.
	 *
	 * This function is largely the same as in GraphicsTest, but now is applied to our BMP test.
	 */
	void compareWithBaseline(const OwningHandle<HostImage>& hostImageHandle, const std::filesystem::path& baselinePath)
	{
		HostImage& hostImage = hostImageHandle.get();
		const uint8_t* pixels = static_cast<const uint8_t*>(hostImage.map());

		// recursively generate the path
		if (!std::filesystem::exists(baselinePath.parent_path()))
		{
			std::filesystem::create_directories(baselinePath.parent_path());
		}

		// Generate a baseline if it doesn't exist
		if (!std::filesystem::exists(baselinePath) || FORCE_REGENERATE_BASELINE)
		{
			unsigned error = 0;
			if (hostImage.getFormat() == ImageFormat::R8)
			{
				error = lodepng::encode(baselinePath.string().c_str(), pixels, hostImage.getWidth(), hostImage.getHeight(),
										LodePNGColorType::LCT_GREY, 8);
			}
			else if (hostImage.getFormat() == ImageFormat::R8G8B8)
			{
				error = lodepng::encode(baselinePath.string().c_str(), pixels, hostImage.getWidth(), hostImage.getHeight(),
										LodePNGColorType::LCT_RGB, 8);
			}
			else if (hostImage.getFormat() == ImageFormat::R8G8B8A8)
			{
				error = lodepng::encode(baselinePath.string().c_str(), pixels, hostImage.getWidth(), hostImage.getHeight(),
										LodePNGColorType::LCT_RGBA, 8);
			}
			else
			{
				FAIL() << "Unsupported image format.";
			}

			EXPECT_EQ(error, 0) << "Failed to save baseline image.";
		}
		else
		{
			// Determine the expected LodePNG color type and number of channels based on the HostImage format.
			LodePNGColorType decodeColorType;
			unsigned expectedChannels = 0;
			if (hostImage.getFormat() == ImageFormat::R8)
			{
				decodeColorType = LCT_GREY;
				expectedChannels = 1;
			}
			else if (hostImage.getFormat() == ImageFormat::R8G8B8)
			{
				decodeColorType = LCT_RGB;
				expectedChannels = 3;
			}
			else if (hostImage.getFormat() == ImageFormat::R8G8B8A8)
			{
				decodeColorType = LCT_RGBA;
				expectedChannels = 4;
			}
			else
			{
				FAIL() << "Unsupported image format.";
			}

			// Load the baseline image with the appropriate color type.
			std::vector<unsigned char> baseline;
			uint32_t width = 0, height = 0;
			unsigned error = lodepng::decode(baseline, width, height, baselinePath.string().c_str(), decodeColorType, 8);
			EXPECT_EQ(error, 0) << "Failed to load baseline image.";

			// Compare dimensions.
			ASSERT_EQ(width, hostImage.getExtent().x) << "Image width mismatch.";
			ASSERT_EQ(height, hostImage.getExtent().y) << "Image height mismatch.";

			// Compute the expected data size.
			size_t expectedDataSize = hostImage.getExtent().x * hostImage.getExtent().y * expectedChannels;
			ASSERT_EQ(baseline.size(), expectedDataSize) << "Image data size mismatch.";

			// Assume 'pixels' is the pointer to the HostImage data.
			for (size_t i = 0; i < baseline.size(); i += expectedChannels)
			{
				// Compute pixel coordinate (optional, for better error messages).
				std::size_t pixelIndex = i / expectedChannels;
				std::size_t x = pixelIndex % hostImage.getExtent().x;
				std::size_t y = pixelIndex / hostImage.getExtent().x;
				for (unsigned channel = 0; channel < expectedChannels; channel++)
				{
					ASSERT_EQ(baseline[i + channel], pixels[i + channel])
						<< "Pixel mismatch at (x=" << x << ", y=" << y << ") channel " << channel;
				}
			}
		}

		hostImage.unmap();
	}
};

TEST_F(BMPTest, TestLoadBMP)
{
	// Specify the BMP file path; assume it�s located in the Data directory
	ImageLoadParams params;
	ImageFile imageFile = _engine->getEngineContext().getResourceSystem().getImageBMPFileProcessor().load("dosFont", dosFont, DOSFONT_SIZE, params);

	ASSERT_TRUE(imageFile.hasImage()) << "Image should be able to load";

	OwningHandle<HostImage> hostImage = imageFile.takeImage();
	ASSERT_TRUE(hostImage.isValid()) << "Failed to load BMP file.";

	OwningHandle<Palette> paletteHandle = imageFile.takePalette();
	ASSERT_FALSE(paletteHandle.isValid()) << "Loaded palette, even though we told it not to";

	// Compare the loaded image with a baseline
	std::filesystem::path baselinePath = _dataPath / "generated" / "BMP" / "test.bmp.png";
	compareWithBaseline(hostImage, baselinePath);
}


TEST_F(BMPTest, TestLoadBMP1)
{
	// Specify the BMP file path; assume it�s located in the Data directory
	ImageLoadParams params;
	ImageFile imageFile = _engine->getEngineContext().getResourceSystem().getImageBMPFileProcessor().load("dosFont", dosFont, DOSFONT_SIZE, params);

	ASSERT_TRUE(imageFile.hasImage()) << "Image should be able to load";

	OwningHandle<HostImage> hostImage = imageFile.takeImage();
	ASSERT_TRUE(hostImage.isValid()) << "Failed to load BMP file.";

	OwningHandle<Palette> paletteHandle = imageFile.takePalette();
	ASSERT_FALSE(paletteHandle.isValid()) << "Loaded palette, even though we told it not to";

	//multiply each value in the image by 255
	uint8_t* pixels = static_cast<uint8_t*>(hostImage->map());

	for (uint32_t i = 0; i < hostImage->getWidth() * hostImage->getHeight(); i++)
	{
		pixels[i] *= 255;
	}

	hostImage->unmap();

	// Compare the loaded image with a baseline
	std::filesystem::path baselinePath = _dataPath / "generated" / "BMP" / "test1.bmp.png";
	compareWithBaseline(hostImage, baselinePath);
}
