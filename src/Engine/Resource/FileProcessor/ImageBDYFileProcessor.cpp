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
#include "ImageBDYFileProcessor.h"

#include "ImageFile.h"

#include "../../EngineContext.h"
#include "../../Filesystem/FileSystem.h"

#include "../ResourceSystem.h"
#include "../../Graphics/Image/Image.h"
#include "../../Graphics/Image/ImageManager.h"
#include "../../Graphics/Palette/Palette.h"
#include "../../Graphics/Palette/PaletteManager.h"

namespace OpenXcom
{

ImageBDYFileProcessor::ImageBDYFileProcessor(EngineContext& context)
	: _context(context)
{
}

ImageBDYFileProcessor::~ImageBDYFileProcessor()
{
}

ImageFile ImageBDYFileProcessor::load(const std::string& name, const std::unique_ptr<FileEntry>& file, const ImageBDYLoadParams& params)
{
	assert(file);

	// Open the file stream.
	std::unique_ptr<std::istream> stream = file->openRead();

	std::vector<uint8_t> decompressed;
	decompressed.reserve(params.extent.x * params.extent.y);

	int x = 0, y = 0;
	while (!stream->eof() && y < params.extent.y) {
		uint8_t command;
		stream->read(reinterpret_cast<char*>(&command), 1);
		if (!stream)
			break; // error or EOF

		if (command >= 129) {
			// Repeated block.
			int count = 257 - command;
			uint8_t pixel;
			stream->read(reinterpret_cast<char*>(&pixel), 1);
			int currentRow = y;
			for (int i = 0; i < count; ++i) {
				// Stop if writing would cross into the next row.
				if (y != currentRow)
					break;
				decompressed.push_back(pixel);
				++x;
				if (x >= params.extent.x) {
					x = 0;
					++y;
				}
			}
		} else {
			// Literal block.
			int count = command + 1;
			int currentRow = y;
			for (int i = 0; i < count; ++i) {
				// Stop if writing would cross into the next row.
				if (y != currentRow)
					break;
				uint8_t pixel;
				stream->read(reinterpret_cast<char*>(&pixel), 1);
				decompressed.push_back(pixel);
				++x;
				if (x >= params.extent.x) {
					x = 0;
					++y;
				}
			}
		}
	}

	// Create the image.
	ImageManager& imageManager = _context.getResourceSystem().getImageManager();
	OwningHandle<HostImage> hostImageHandle = imageManager.createHostImage(name, params.extent, ImageFormat::R8);
	HostImage& hostImage = hostImageHandle.get();

	// Copy the data to the image.
	uint8_t* dstImageData = static_cast<uint8_t*>(hostImage.map());
	std::copy(decompressed.begin(), decompressed.end(), dstImageData);
	hostImage.unmap();

	return ImageFile(std::move(hostImageHandle), OwningHandle<Palette>());
}

} // namespace OpenXcom
