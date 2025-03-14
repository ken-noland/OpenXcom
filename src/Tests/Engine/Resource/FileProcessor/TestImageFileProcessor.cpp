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

#include "../../../../Engine/Resource/FileProcessor/ImageFileProcessor.h"
#include "../../../../Engine/Resource/FileProcessor/ImageBMPFileProcessor.h"
#include "../../../../Engine/Resource/FileProcessor/ImagePNGFileProcessor.h"

using namespace OpenXcom;

class ImageFileProcessorTest : public TestEngineSuite
{
protected:
};

TEST_F(ImageFileProcessorTest, TestBmp8bit)
{
	ImagePaletteFile generatedImage = createImage();
	ASSERT_TRUE(generatedImage.image.isValid()) << "Image handle should be valid";
	ASSERT_TRUE(generatedImage.palette.isValid()) << "Palette handle should be valid";
	ASSERT_EQ(generatedImage.image->getFormat(), ImageFormat::R8) << "Generated image format should be R8";

	ImageBMPFileProcessor& bmpProcessor = _engine->getEngineContext().getResourceSystem().getImageBMPFileProcessor();

	// Save the image
	std::filesystem::path path = _dataPath / "generated" / "BMP" / "001_generated.bmp";

	// recursively generate the path
	if (!std::filesystem::exists(path.parent_path()))
	{
		std::filesystem::create_directories(path.parent_path());
	}

	bmpProcessor.save(path, generatedImage);

	// Load the image back
	ImageLoadParams params;
	ImagePaletteFile loadedImage = bmpProcessor.load("loaded_bmp_bitmap", path, params);
	ASSERT_TRUE(loadedImage.image) << "Unable to load bitmap";

	ASSERT_TRUE(loadedImage.image.isValid()) << "Image handle should be valid";
	ASSERT_FALSE(loadedImage.palette.isValid()) << "Palette handle should not be valid(we didn't request it to load)";

	// Check image format
	ASSERT_EQ(loadedImage.image->getFormat(), ImageFormat::R8) << "Image format mismatch";

	// Check the image data
	HostImage& loadedHostImage = *loadedImage.image;
	const uint8_t* loadedPixels = static_cast<const uint8_t*>(loadedHostImage.map());
	const uint8_t* generatedPixels = static_cast<const uint8_t*>(generatedImage.image->map());

	for (uint32_t y = 0; y < loadedHostImage.getHeight(); y++)
	{
		for (uint32_t x = 0; x < loadedHostImage.getWidth(); x++)
		{
			ASSERT_EQ(loadedPixels[y * loadedHostImage.getWidth() + x], generatedPixels[y * loadedHostImage.getWidth() + x]) << "Pixel mismatch at (" << x << ", " << y << ")";
		}
	}

	loadedHostImage.unmap();
	generatedImage.image->unmap();
}


TEST_F(ImageFileProcessorTest, TestPng8bit)
{
	ImagePaletteFile generatedImage = createImage();
	ASSERT_TRUE(generatedImage.image.isValid()) << "Image handle should be valid";
	ASSERT_TRUE(generatedImage.palette.isValid()) << "Palette handle should be valid";
	ASSERT_EQ(generatedImage.image->getFormat(), ImageFormat::R8) << "Generated image format should be R8";

	ImagePNGFileProcessor& pngProcessor = _engine->getEngineContext().getResourceSystem().getImagePNGFileProcessor();

	// Save the image
	std::filesystem::path path = _dataPath / "generated" / "PNG" / "001_generated.png";

	// recursively generate the path
	if (!std::filesystem::exists(path.parent_path()))
	{
		std::filesystem::create_directories(path.parent_path());
	}

	pngProcessor.save(path, generatedImage);

	// Load the image back
	ImageLoadParams params;
	ImagePaletteFile loadedImage = pngProcessor.load("loaded_bmp_bitmap", path, params);
	ASSERT_TRUE(loadedImage.image) << "Unable to load bitmap";

	ASSERT_TRUE(loadedImage.image.isValid()) << "Image handle should be valid";
	ASSERT_FALSE(loadedImage.palette.isValid()) << "Palette handle should not be valid(we didn't request it to load)";

	// Check image format
	ASSERT_EQ(loadedImage.image->getFormat(), ImageFormat::R8) << "Image format mismatch";

	// Check the image data
	HostImage& loadedHostImage = *loadedImage.image;
	const uint8_t* loadedPixels = static_cast<const uint8_t*>(loadedHostImage.map());
	const uint8_t* generatedPixels = static_cast<const uint8_t*>(generatedImage.image->map());

	for (uint32_t y = 0; y < loadedHostImage.getHeight(); y++)
	{
		for (uint32_t x = 0; x < loadedHostImage.getWidth(); x++)
		{
			ASSERT_EQ(loadedPixels[y * loadedHostImage.getWidth() + x], generatedPixels[y * loadedHostImage.getWidth() + x]) << "Pixel mismatch at (" << x << ", " << y << ")";
		}
	}

	loadedHostImage.unmap();
	generatedImage.image->unmap();
}
