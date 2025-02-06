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
#include "ImagePNGFileProcessor.h"

#include "ImageFile.h"

#include "../../EngineContext.h"

#include "../ResourceSystem.h"
#include "../../Graphics/Palette/Palette.h"
#include "../../Graphics/Palette/PaletteManager.h"
#include "../../Graphics/Image/Image.h"
#include "../../Graphics/Image/ImageManager.h"
#include "../../Graphics/Buffer/Buffer.h"
#include "../../Graphics/Buffer/BufferManager.h"


#include <lodepng.h>

namespace OpenXcom
{

ImagePNGFileProcessor::ImagePNGFileProcessor(EngineContext& context)
	: _context(context)
{

}

ImagePNGFileProcessor::~ImagePNGFileProcessor()
{
}

ImageFile ImagePNGFileProcessor::load(const std::string& name, const std::filesystem::path& filename, bool loadPalette)
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	BufferManager& bufferManager = resourceSystem.getBufferManager();
	PaletteManager& paletteManager = resourceSystem.getPaletteManager();
	ImageManager& imageManager = resourceSystem.getImageManager();

	// --- lodepng Setup ---
	// We use the C++ interface and set up a State object.
	lodepng::State state;
	state.decoder.color_convert = 0;

	// --- Load the PNG ---
	std::vector<unsigned char> buffer;
	unsigned error = lodepng::load_file(buffer, filename.string());
	if (error)
	{
		throw std::runtime_error("Failed to load PNG file \"" + filename.string() + "\" with error: " + lodepng_error_text(error));
	}

	// --- Decode the PNG ---
	std::vector<unsigned char> image;
	unsigned width, height;
	error = lodepng::decode(image, width, height, state, buffer);
	if (error)
	{
		throw std::runtime_error("Failed to decode PNG file \"" + filename.string() + "\" with error: " + lodepng_error_text(error));
	}

	// For now, only support 8-bit (R8) images.
	assert(state.info_png.color.colortype == LCT_PALETTE);
	assert(state.info_png.color.bitdepth == 8);
	assert(state.info_raw.colortype == LCT_PALETTE);
	assert(state.info_raw.bitdepth == 8);

	// --- Create the Image ---
	OwningHandle<HostImage> hostImageHandle = imageManager.createHostImage(name, glm::ivec2(width, height), ImageFormat::R8);
	uint8_t* imageDataPtr = static_cast<uint8_t*>(hostImageHandle->map());
	std::copy(image.begin(), image.end(), imageDataPtr);
	hostImageHandle->unmap();

	// --- Create the Palette ---
	OwningHandle<Palette> paletteHandle;

	if(loadPalette)
	{
		std::vector<PackedColor> paletteData;
		paletteData.resize(state.info_png.color.palettesize);

		// Copy the palette data from the state object.
		for (int i = 0; i < state.info_png.color.palettesize; i++)
		{
			paletteData[i] = PackedColor(state.info_png.color.palette[i * 4 + 0], state.info_png.color.palette[i * 4 + 1], state.info_png.color.palette[i * 4 + 2]);
		}

		// Create the Palette object.
		paletteHandle = paletteManager.createPalette(name + "_png_palette", paletteData.data(), paletteData.size());
	}

	return ImageFile(std::move(hostImageHandle), std::move(paletteHandle));
}

bool ImagePNGFileProcessor::save(const std::filesystem::path& filename, ImageFile& imageData)
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	BufferManager& bufferManager = resourceSystem.getBufferManager();

	HostImage& hostImage = imageData.getImage();
	DeviceBuffer& devicePaletteBuffer = imageData.getPalette().getDeviceBuffer();
	std::unique_ptr<HostBuffer> hostPaletteBufferPtr = bufferManager.createHostBuffer(devicePaletteBuffer);
	HostBuffer& hostPaletteBuffer = *hostPaletteBufferPtr;

	// For now, only support 8-bit (R8) images.
	assert(hostImage.getFormat() == ImageFormat::R8);
	assert(hostPaletteBuffer.getElementSize() == sizeof(PackedColor));

	// --- lodepng Setup ---
	// We use the C++ interface and set up a State object.
	lodepng::State state;
	// Specify that both the PNG output and the raw image data use a palette.
	state.info_png.color.colortype = LCT_PALETTE;
	state.info_png.color.bitdepth = 8;
	state.info_raw.colortype = LCT_PALETTE;
	state.info_raw.bitdepth = 8;
	state.encoder.auto_convert = 0; // Disable automatic conversion

	PackedColor* paletteData = static_cast<PackedColor*>(hostPaletteBuffer.map());

	// For indices 1 to 255, generate colors via an HSV color wheel.
	for (int i = 0; i < hostPaletteBuffer.getCount(); i++)
	{
		PackedColor c = paletteData[i];
		lodepng_palette_add(&state.info_png.color, c.r(), c.g(), c.b(), 255);
		lodepng_palette_add(&state.info_raw, c.r(), c.g(), c.b(), 255);
	}

	hostPaletteBuffer.unmap();

	//get the image pixels
	const uint8_t* imageDataPtr = static_cast<const uint8_t*>(hostImage.map());

	// --- Encode and Save the PNG ---
	std::vector<unsigned char> buffer;
	unsigned error = lodepng::encode(buffer, imageDataPtr, hostImage.getWidth(), hostImage.getHeight(), state);
	if (error)
	{
		throw std::runtime_error("Failed to save PNG file \"" + filename.string() + "\" with error: " + lodepng_error_text(error));
	}

	error = lodepng::save_file(buffer, filename.string());
	if (error)
	{
		throw std::runtime_error("Failed to save PNG file \"" + filename.string() + "\" with error: " + lodepng_error_text(error));
	}

	hostImage.unmap();

	return true;
}


} // namespace OpenXcom
