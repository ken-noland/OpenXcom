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
#include "ImagePCKFileProcessor.h"

#include "ImageFile.h"

#include "../../EngineContext.h"
#include "../../Filesystem/FileSystem.h"

#include "../ResourceSystem.h"
#include "../../Graphics/Image/Image.h"
#include "../../Graphics/Image/ImageManager.h"

namespace OpenXcom
{

ImagePCKFileProcessor::ImagePCKFileProcessor(EngineContext& context)
	: _context(context)
{
}

ImagePCKFileProcessor::~ImagePCKFileProcessor()
{
}

ImageSetFile ImagePCKFileProcessor::load(const std::string& name,
                                          const std::unique_ptr<FileEntry>& pckFile,
                                          const std::unique_ptr<FileEntry>& tabFile,
                                          const ImagePCKLoadParams& params)
{
    // Determine the number of frames using the TAB file if it exists.
    int numFrames = 0;
    if (tabFile && tabFile->getSize() > 0) {
        std::unique_ptr<std::istream> offsetStream = tabFile->openRead();
        std::streampos beginPos = offsetStream->tellg();
        int offset = 0;
        offsetStream->read(reinterpret_cast<char*>(&offset), sizeof(offset));
        offsetStream->seekg(0, std::ios::end);
        std::streampos endPos = offsetStream->tellg();
        int size = static_cast<int>(endPos - beginPos);
        // If the first offset is nonzero, then offsets are 16-bit.
        if (offset != 0) {
            numFrames = size / 2;
        } else {
            numFrames = size / 4;
        }
    } else {
        numFrames = 1;
    }

    // Create a vector to hold the image handles.
    std::vector<OwningHandle<HostImage>> imageHandles;
    imageHandles.reserve(numFrames);

    // Open the PCK file stream.
    std::unique_ptr<std::istream> pckStream = pckFile->openRead();

    // Process each frame.
    for (int frameIndex = 0; frameIndex < numFrames; ++frameIndex) {
        // Create a HostImage for this frame.
        ImageManager& imageManager = _context.getResourceSystem().getImageManager();
        OwningHandle<HostImage> hostImageHandle = imageManager.createHostImage(
            name + "_" + std::to_string(frameIndex),
            params.extent,
            ImageFormat::R8);
        HostImage& hostImage = hostImageHandle.get();

        // Prepare a decompression buffer.
        std::vector<uint8_t> decompressed;
        decompressed.reserve(params.extent.x * params.extent.y);

        // (x, y) coordinates to simulate setPixelIterative behavior.
        int x = 0;
        int y = 0;

        // First, read the header byte that tells how many full rows to fill with zeroes.
        uint8_t header = 0;
        pckStream->read(reinterpret_cast<char*>(&header), sizeof(header));
        for (int row = 0; row < header; ++row) {
            for (int col = 0; col < params.extent.x; ++col) {
                decompressed.push_back(0);
                ++x;
                if (x >= params.extent.x) {
                    x = 0;
                    ++y;
                }
            }
        }

        // Then, read pixel data until the terminator (255) is encountered.
        uint8_t value = 0;
        while (pckStream->read(reinterpret_cast<char*>(&value), sizeof(value)) && value != 255) {
            if (value == 254) {
                // 254 indicates that the next byte is a count of consecutive zeroes.
                uint8_t count = 0;
                pckStream->read(reinterpret_cast<char*>(&count), sizeof(count));
                for (int i = 0; i < count; ++i) {
                    decompressed.push_back(0);
                    ++x;
                    if (x >= params.extent.x) {
                        x = 0;
                        ++y;
                    }
                }
            } else {
                // Otherwise, write the read value as a pixel.
                decompressed.push_back(value);
                ++x;
                if (x >= params.extent.x) {
                    x = 0;
                    ++y;
                }
            }
        }

        // Copy the decompressed data into the HostImage.
        uint8_t* destinationData = static_cast<uint8_t*>(hostImage.map());
        std::copy(decompressed.begin(), decompressed.end(), destinationData);
        hostImage.unmap();

        // Add this image handle to the collection.
        imageHandles.push_back(std::move(hostImageHandle));
    }

    // Return the ImageSetFile containing all the image handles; palette is left empty.
    return ImageSetFile{std::move(imageHandles)};
}



} // namespace OpenXcom
