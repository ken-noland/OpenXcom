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
#include "ImageDATFileProcessor.h"

#include "ImageFile.h"

#include "../../EngineContext.h"
#include "../../Filesystem/FileSystem.h"

#include "../ResourceSystem.h"
#include "../../Graphics/Image/Image.h"
#include "../../Graphics/Image/ImageManager.h"

namespace OpenXcom
{

ImageDATFileProcessor::ImageDATFileProcessor(EngineContext& context)
	: _context(context)
{
}

ImageDATFileProcessor::~ImageDATFileProcessor()
{
}

ImageSetFile ImageDATFileProcessor::load(const std::string& name,
                                          const std::unique_ptr<FileEntry>& datFile,
                                          const ImageDATLoadParams& params)
{
    // Open the DAT file stream.
    std::unique_ptr<std::istream> datStream = datFile->openRead();

    // Seek to the end to determine the file size.
    datStream->seekg(0, std::ios::end);
    std::streamoff fileSize = datStream->tellg();
    datStream->seekg(0, std::ios::beg);

    // Determine frame dimensions from load parameters.
    int frameWidth  = params.extent.x;
    int frameHeight = params.extent.y;
    int pixelsPerFrame = frameWidth * frameHeight;

    // Calculate the number of frames in the DAT file.
    int numFrames = static_cast<int>(fileSize) / pixelsPerFrame;

    // Prepare a vector to hold image handles.
    std::vector<OwningHandle<HostImage>> imageHandles;
    imageHandles.reserve(numFrames);

    // Create a HostImage for each frame.
    for (int frameIndex = 0; frameIndex < numFrames; ++frameIndex) {
        ImageManager& imageManager = _context.getResourceSystem().getImageManager();
        OwningHandle<HostImage> hostImageHandle = imageManager.createHostImage(
            name + "_" + std::to_string(frameIndex),
            params.extent,
            ImageFormat::R8);
        imageHandles.push_back(std::move(hostImageHandle));
    }

    // Temporary buffer for one frame of pixel data.
    std::vector<uint8_t> frameData;
    frameData.resize(pixelsPerFrame);

    // These variables simulate the setPixelIterative behavior.
    int x = 0;
    int y = 0;
    int pixelIndex = 0; // Position in the current frame's buffer.
    int currentFrame = 0;

    // Read one pixel (byte) at a time from the file.
    uint8_t pixelValue = 0;
    while (datStream->read(reinterpret_cast<char*>(&pixelValue), sizeof(pixelValue))) {
        // Store the pixel value into our temporary frame buffer.
        frameData[pixelIndex] = pixelValue;
        ++pixelIndex;
        ++x;

        // When we reach the end of a row, reset x and increment y.
        if (x >= frameWidth) {
            x = 0;
            ++y;
        }

        // If the full frame has been filled, copy it into the image.
        if (y >= frameHeight) {
            OwningHandle<HostImage>& currentImageHandle = imageHandles[currentFrame];
            HostImage& hostImage = currentImageHandle.get();
            uint8_t* destinationData = static_cast<uint8_t*>(hostImage.map());
            std::copy(frameData.begin(), frameData.end(), destinationData);
            hostImage.unmap();

            // Prepare for the next frame.
            currentFrame++;
            if (currentFrame >= numFrames) {
                break;
            }
            x = 0;
            y = 0;
            pixelIndex = 0;
        }
    }

    // Return the new ImageSetFile, with the palette left empty.
    return ImageSetFile(std::move(imageHandles));
}




} // namespace OpenXcom
