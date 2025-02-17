#pragma once
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
#include "../Handle.h"

namespace OpenXcom
{

class HostImage;
class Palette;

struct ImageLoadParams
{
	bool loadPalette = false;
};
;

// This class represents an image that is being read from disk. It contains the image
// and palette handles, and is used to transfer ownership of these resources between
// the file processor and the resource manager.
class ImageFile
{
protected:
	OwningHandle<HostImage> _image;
	OwningHandle<Palette> _palette;

public:
	// Blank constructor: Invalid handle
	ImageFile();

	// Constructor with image and palette handles
	ImageFile(OwningHandle<HostImage> image, OwningHandle<Palette> palette);

	// Move constructor: Transfers ownership
	ImageFile(ImageFile&& other);

	// Destructor
	~ImageFile();

	// Move assignment: Transfers ownership
	ImageFile& operator=(ImageFile&& other);

	// Delete copy constructor and copy assignment
	ImageFile(const ImageFile&) = delete;
	ImageFile& operator=(const ImageFile&) = delete;

	bool hasImage() const;
	bool hasPalette() const;

	HostImage& getImage();
	Palette& getPalette();

	ResourceHandle<HostImage> getImageHandle();
	ResourceHandle<Palette> getPaletteHandle();

	// Transfers ownership of the image and invalidates the internal handle.
	OwningHandle<HostImage> takeImage();

	// Transfers ownership of the palette and invalidates the internal handle.
	OwningHandle<Palette> takePalette();

};

} // namespace OpenXcom
