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
#include "ImageSPKFileProcessor.h"

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

ImageSPKFileProcessor::ImageSPKFileProcessor(EngineContext& context)
	: _context(context)
{
}

ImageSPKFileProcessor::~ImageSPKFileProcessor()
{
}

ImageFile ImageSPKFileProcessor::load(const std::string& name, const std::unique_ptr<FileEntry>& file, const ImageSPKLoadParams& params)
{
    assert(file);

    // Open the file stream.
    std::unique_ptr<std::istream> stream = file->openRead();

    // Decompress the SPK file data into a vector.
    std::vector<uint8_t> decompressed;
    decompressed.reserve(params.extent.x * params.extent.y);

    int x = 0, y = 0;
    while (stream->peek() != EOF && y < params.extent.y) {
        // Read a 16-bit flag in little endian.
        uint16_t flag;
        stream->read(reinterpret_cast<char*>(&flag), sizeof(uint16_t));
        if (!stream)
            break; // Error or unexpected EOF.

        if (flag == 0xFFFF) {
            // For flag 0xFFFF, read the count, then write (count * 2) pixels of 0.
            uint16_t count;
            stream->read(reinterpret_cast<char*>(&count), sizeof(uint16_t));
            int totalPixels = count * 2;
            for (int i = 0; i < totalPixels; ++i) {
                decompressed.push_back(0);
                ++x;
                if (x >= params.extent.x) {
                    x = 0;
                    ++y;
                    if (y >= params.extent.y)
                        break;
                }
            }
        } else if (flag == 0xFFFE) {
            // For flag 0xFFFE, read the count, then write (count * 2) literal pixels.
            uint16_t count;
            stream->read(reinterpret_cast<char*>(&count), sizeof(uint16_t));
            int totalPixels = count * 2;
            for (int i = 0; i < totalPixels; ++i) {
                uint8_t pixel;
                stream->read(reinterpret_cast<char*>(&pixel), 1);
                decompressed.push_back(pixel);
                ++x;
                if (x >= params.extent.x) {
                    x = 0;
                    ++y;
                    if (y >= params.extent.y)
                        break;
                }
            }
        }
    }

    // Create the image.
    ImageManager& imageManager = _context.getResourceSystem().getImageManager();
    OwningHandle<HostImage> hostImageHandle = imageManager.createHostImage(name, params.extent, ImageFormat::R8);
    HostImage& hostImage = hostImageHandle.get();

    // Copy the decompressed data to the image.
    uint8_t* dstImageData = static_cast<uint8_t*>(hostImage.map());
    std::copy(decompressed.begin(), decompressed.end(), dstImageData);
    hostImage.unmap();

    return ImageFile{std::move(hostImageHandle)};
}

} // namespace OpenXcom
