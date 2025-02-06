#include "ImageBMPFileProcessor.h"
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

#include "../../EngineContext.h"

#include "../ResourceSystem.h"
#include "../../Graphics/Palette/Palette.h"
#include "../../Graphics/Palette/PaletteManager.h"
#include "../../Graphics/Image/Image.h"
#include "../../Graphics/Image/ImageManager.h"

#include <glm/vec2.hpp>

extern "C" {
#include <libbmp.h>
}

namespace OpenXcom
{

// TODO: maybe grab https://github.com/jsummers/bmpsuite, which is a utility to generate tons of bitmaps(some of them invalid on purpose) and test this code with it


// Base template (will never be used directly)
template <int Bits>
struct PixelUnpacker
{
	// Unpack from BMP's pixel data into dst (host image buffer).
	// rowBytes: the number of bytes per row in the BMP data (not including padding)
	// padding: the extra bytes at the end of each row.
	// bottomUp: whether the image is stored bottom-up.
	static void unpack(const unsigned char* pixelData, int width, int height,
					   int rowBytes, int padding, bool bottomUp, uint8_t* dst)
	{
		static_assert(Bits == 1 || Bits == 4 || Bits == 8 || Bits == 24 || Bits == 32,
					  "Unsupported bit depth.");
	}
};

// Specialization for 1-bit images (unpack into one byte per pixel holding a palette index).
template <>
struct PixelUnpacker<1>
{
	static void unpack(const unsigned char* pixelData, int width, int height,
					   int rowBytes, int padding, bool bottomUp, uint8_t* dst)
	{
		// Each row in the BMP is rowBytes (computed as (width+7)/8) plus 'padding' bytes.
		for (int row = 0; row < height; row++)
		{
			int actualRow = bottomUp ? (height - 1 - row) : row;
			const unsigned char* rowPtr = pixelData + actualRow * (rowBytes + padding);
			for (int col = 0; col < width; col++)
			{
				int byteIndex = col / 8;
				int bitIndex = 7 - (col % 8); // BMP uses MSB first.
				uint8_t bit = (rowPtr[byteIndex] >> bitIndex) & 0x01;
				dst[row * width + col] = bit;
			}
		}
	}
};

// Specialization for 4-bit images (each byte contains 2 pixels).
template <>
struct PixelUnpacker<4>
{
	static void unpack(const unsigned char* pixelData, int width, int height,
					   int rowBytes, int padding, bool bottomUp, uint8_t* dst)
	{
		// rowBytes is (width + 1)/2.
		for (int row = 0; row < height; row++)
		{
			int actualRow = bottomUp ? (height - 1 - row) : row;
			const unsigned char* rowPtr = pixelData + actualRow * (rowBytes + padding);
			int dstIndex = row * width;
			for (int col = 0; col < width; col++)
			{
				int byteIndex = col / 2;
				bool highNibble = (col % 2 == 0);
				uint8_t nibble = highNibble ? (rowPtr[byteIndex] >> 4) & 0x0F
											: rowPtr[byteIndex] & 0x0F;
				dst[dstIndex + col] = nibble;
			}
		}
	}
};

// Specialization for 8-bit images (direct copy, one byte per pixel).
template <>
struct PixelUnpacker<8>
{
	static void unpack(const unsigned char* pixelData, int width, int height,
					   int rowBytes, int padding, bool bottomUp, uint8_t* dst)
	{
		// For 8-bit images, rowBytes equals the width.
		for (int row = 0; row < height; row++)
		{
			int actualRow = bottomUp ? (height - 1 - row) : row;
			const unsigned char* srcRow = pixelData + actualRow * (rowBytes + padding);
			std::memcpy(dst + row * width, srcRow, width);
		}
	}
};

// Specialization for 24-bit images (BGR to RGB conversion).
template <>
struct PixelUnpacker<24>
{
	static void unpack(const unsigned char* pixelData, int width, int height,
					   int rowBytes, int padding, bool bottomUp, uint8_t* dst)
	{
		// For 24-bit, each pixel is 3 bytes.
		int bytesPerPixel = 3;
		for (int row = 0; row < height; row++)
		{
			int actualRow = bottomUp ? (height - 1 - row) : row;
			const unsigned char* srcRow = pixelData + actualRow * (rowBytes + padding);
			uint8_t* dstRow = dst + row * width * bytesPerPixel;
			for (int col = 0; col < width; col++)
			{
				const unsigned char* srcPixel = srcRow + col * bytesPerPixel;
				// BMP stores pixels as BGR; convert to RGB.
				dstRow[col * 3 + 0] = srcPixel[2];
				dstRow[col * 3 + 1] = srcPixel[1];
				dstRow[col * 3 + 2] = srcPixel[0];
			}
		}
	}
};

// Specialization for 32-bit images (BGRA to RGBA conversion).
template <>
struct PixelUnpacker<32>
{
	static void unpack(const unsigned char* pixelData, int width, int height,
					   int rowBytes, int padding, bool bottomUp, uint8_t* dst)
	{
		// For 32-bit, each pixel is 4 bytes.
		int bytesPerPixel = 4;
		for (int row = 0; row < height; row++)
		{
			int actualRow = bottomUp ? (height - 1 - row) : row;
			const unsigned char* srcRow = pixelData + actualRow * (rowBytes + padding);
			uint8_t* dstRow = dst + row * width * bytesPerPixel;
			for (int col = 0; col < width; col++)
			{
				const unsigned char* srcPixel = srcRow + col * bytesPerPixel;
				// BMP typically stores as BGRA.
				dstRow[col * 4 + 0] = srcPixel[2];
				dstRow[col * 4 + 1] = srcPixel[1];
				dstRow[col * 4 + 2] = srcPixel[0];
				dstRow[col * 4 + 3] = srcPixel[3];
			}
		}
	}
};



ImageBMPFileProcessor::ImageBMPFileProcessor(EngineContext& context)
	: _context(context)
{
}

ImageBMPFileProcessor::~ImageBMPFileProcessor()
{
}

std::pair<OwningHandle<HostImage>, OwningHandle<Palette>> ImageBMPFileProcessor::load(const std::filesystem::path& filename, bool loadPalette)
{
	return std::pair<OwningHandle<HostImage>, OwningHandle<Palette>>();
}

std::pair<OwningHandle<HostImage>, OwningHandle<Palette>> ImageBMPFileProcessor::load(const uint8_t* bmpBuffer, std::size_t size, bool loadPalette)
{
	// Verify magic.
	unsigned short magic;
	std::memcpy(&magic, bmpBuffer, sizeof(magic));
	if (magic != BMP_MAGIC)
	{
		return {OwningHandle<HostImage>(), OwningHandle<Palette>()};
	}

	// Read header.
	bmp_header header;
	std::memcpy(&header, bmpBuffer + 2, sizeof(bmp_header));
	int width = header.biWidth;
	int height = std::abs(header.biHeight);
	bool bottomUp = (header.biHeight > 0);

	PaletteManager::OwningHandle paletteHandle;
	if (header.biBitCount <= 8 && loadPalette)
	{
		// Load palette (same as before).
		size_t paletteOffset = 2 + sizeof(bmp_header);
		unsigned int numEntries = header.biClrUsed ? header.biClrUsed : (1 << header.biBitCount);
		std::vector<PackedColor> palette(numEntries);
		for (unsigned int i = 0; i < numEntries; i++)
		{
			const unsigned char* entryPtr = bmpBuffer + paletteOffset + i * 4;
			uint8_t red = entryPtr[2];
			uint8_t green = entryPtr[1];
			uint8_t blue = entryPtr[0];
			uint8_t alpha = 255;
			uint32_t packedColor = (red << 24) | (green << 16) | (blue << 8) | (alpha);
			palette[i] = packedColor;
		}
		PaletteManager& paletteManager = _context.getResourceSystem().getPaletteManager();
		paletteHandle = paletteManager.createPalette("BMP", palette.data(), numEntries);
	}

	// Create a HostImage with an appropriate image format.
	glm::ivec2 imageSize(width, height);
	ImageManager& imageManager = _context.getResourceSystem().getImageManager();
	ImageFormat format;
	int bytesPerPixelFile = 0;
	switch (header.biBitCount)
	{
	case 1:
	case 4:
	case 8:
		format = ImageFormat::R8;
		bytesPerPixelFile = 1; // Not per-pixel in the file, but we'll unpack into one byte per pixel.
		break;
	case 24:
		format = ImageFormat::R8G8B8;
		bytesPerPixelFile = 3;
		break;
	case 32:
		format = ImageFormat::R8G8B8A8;
		bytesPerPixelFile = 4;
		break;
	default:
		throw std::runtime_error("Unsupported bit depth");
	}
	OwningHandle<HostImage> hostImageHandle = imageManager.createHostImage(imageSize, format);
	if (!hostImageHandle.isValid())
	{
		return {OwningHandle<HostImage>(), OwningHandle<Palette>()};
	}
	HostImage& hostImage = hostImageHandle.get();

	uint8_t* dstImageData = static_cast<uint8_t*>(hostImage.map());

	// Compute row sizes and padding from the file.
	int rowBytesFile = 0;
	if (header.biBitCount <= 8)
	{
		// For 1-bit: rowBytes = (width+7)/8; for 4-bit: (width+1)/2; for 8-bit: width.
		if (header.biBitCount == 1)
			rowBytesFile = (width + 7) / 8;
		else if (header.biBitCount == 4)
			rowBytesFile = (width + 1) / 2;
		else // 8-bit
			rowBytesFile = width;
	}
	else
	{
		rowBytesFile = width * bytesPerPixelFile;
	}
	int padding = (4 - (rowBytesFile % 4)) % 4;
	const unsigned char* pixelData = bmpBuffer + header.bfOffBits;

	// Call the appropriate unpacker based on bit depth.
	switch (header.biBitCount)
	{
	case 1:
		PixelUnpacker<1>::unpack(pixelData, width, height, rowBytesFile, padding, bottomUp, dstImageData);
		break;
	case 4:
		PixelUnpacker<4>::unpack(pixelData, width, height, rowBytesFile, padding, bottomUp, dstImageData);
		break;
	case 8:
		PixelUnpacker<8>::unpack(pixelData, width, height, rowBytesFile, padding, bottomUp, dstImageData);
		break;
	case 24:
		PixelUnpacker<24>::unpack(pixelData, width, height, rowBytesFile, padding, bottomUp, dstImageData);
		break;
	case 32:
		PixelUnpacker<32>::unpack(pixelData, width, height, rowBytesFile, padding, bottomUp, dstImageData);
		break;
	default:
		// Should not reach here.
		break;
	}
	hostImage.unmap();

	return {std::move(hostImageHandle), std::move(paletteHandle)};
}

} // namespace OpenXcom
