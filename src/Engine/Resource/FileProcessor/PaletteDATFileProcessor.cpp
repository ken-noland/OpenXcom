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
#include "PaletteDATFileProcessor.h"

#include "../ResourceSystem.h"
#include "../../EngineContext.h"
#include "../../Logger.h"
#include "../../Filesystem/FileSystem.h"


namespace OpenXcom
{

constexpr size_t defaultPaletteBlockSize = 768 + 6;

PaletteDATFileProcessor::PaletteDATFileProcessor(EngineContext& context)
	: _context(context)
{
}

PaletteDATFileProcessor::~PaletteDATFileProcessor()
{
}

size_t PaletteDATFileProcessor::getPaletteBlockFileSize(const PaletteLoadParams& params) const
{
	return (params.filePaletteEntries * 3);
}

size_t PaletteDATFileProcessor::getRequiredFileSize(size_t numPalettes, const PaletteLoadParams& params) const
{
	return numPalettes * getPaletteBlockFileSize(params);
}

size_t PaletteDATFileProcessor::getPaletteCount(const std::unique_ptr<FileEntry>& file, const PaletteLoadParams& params) const
{
	return file->getSize() / getPaletteBlockFileSize(params);
}

PaletteFile PaletteDATFileProcessor::load(const std::vector<std::string>& names, const std::unique_ptr<FileEntry>& file, const PaletteLoadParams& params)
{
	assert(file);

	// Open the file stream.
	std::unique_ptr<std::istream> stream = file->openRead();
	if (!stream)
	{
		Log(LOG_ERROR) << "Failed to open palette file for reading: " << file->getPath();
		return PaletteFile();
	}

	// Determine file size.
	stream->seekg(0, std::ios::end);
	std::streampos fileSize = stream->tellg();
	stream->seekg(0, std::ios::beg);

	size_t numPalettes = names.size();
	size_t requiredSize = getRequiredFileSize(numPalettes, params);
	size_t paletteBlockSize = getPaletteBlockFileSize(params);

	if(static_cast<size_t>(fileSize) < requiredSize)
	{
		Log(LOG_ERROR) << "Palette file too small. Required " << requiredSize << " bytes, found " << fileSize;
		return PaletteFile();
	}

	PaletteFile paletteFile;

	// For each palette name provided, read the corresponding data.
	for (size_t i = 0; i < numPalettes; ++i)
	{
		// Compute the offset using the helper function:
		size_t offset = i * paletteBlockSize;
		stream->seekg(offset, std::ios::beg);

		size_t numColors = params.paletteEntries;
		size_t colorDataSize = numColors * 3;

		std::vector<uint8_t> colorBytes(colorDataSize);
		stream->read(reinterpret_cast<char*>(colorBytes.data()), colorDataSize);
		if (stream->gcount() != colorDataSize)
		{
			Log(LOG_ERROR) << "Failed to read color data for palette index " << i;
			continue;
		}

		// Convert raw bytes to an array of PackedColor
		std::vector<PackedColor> colors(numColors);
		for (int j = 0; j < numColors; ++j)
		{
			// Multiply each channel by 4 to convert X-Com format to RGB.
			uint8_t r = colorBytes[j * 3 + 0] * 4;
			uint8_t g = colorBytes[j * 3 + 1] * 4;
			uint8_t b = colorBytes[j * 3 + 2] * 4;
			colors[j].set(r, g, b, 255);
		}

		// The first palette's first entry has its 'alpha' field set to 0.
		colors[0].set(colors[0].r(), colors[0].g(), colors[0].b(), 0);

		// Create the palette using the PaletteManager.
		PaletteManager& paletteManager = _context.getResourceSystem().getPaletteManager();
		// Here, appending "_bmp_palette" mimics the old naming convention.
		OwningHandle<Palette> paletteHandle = paletteManager.createPalette(names[i], colors.data(), numColors);

		// Add the newly created palette to the PaletteFile.
		paletteFile.add(paletteHandle);
	}

	return paletteFile;
}

} // namespace OpenXcom
