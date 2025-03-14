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
#include "ImageSCRFileProcessor.h"

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

ImageSCRFileProcessor::ImageSCRFileProcessor(EngineContext& context)
	: _context(context)
{
}

ImageSCRFileProcessor::~ImageSCRFileProcessor()
{
}

ImageFile ImageSCRFileProcessor::load(const std::string& name, const std::unique_ptr<FileEntry>& file, const ImageSCRLoadParams& params)
{
	assert(file);

	size_t size = params.extent.x * params.extent.y;

	// Open the file stream.
	std::unique_ptr<std::istream> stream = file->openRead();

	// Read the image data.
	std::vector<uint8_t> data(size);
	stream->read(reinterpret_cast<char*>(data.data()), size);

	// Create the image.
	ImageManager& imageManager = _context.getResourceSystem().getImageManager();
	OwningHandle<HostImage> hostImageHandle = imageManager.createHostImage(name, params.extent, ImageFormat::R8);
	HostImage& hostImage = hostImageHandle.get();

	// Copy the data to the image.
	uint8_t* dstImageData = static_cast<uint8_t*>(hostImage.map());
	std::copy(data.begin(), data.end(), dstImageData);
	hostImage.unmap();

	return ImageFile{std::move(hostImageHandle)};
}

} // namespace OpenXcom
