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

class ImageFile
{
protected:
	OwningHandle<HostImage> _image;
	OwningHandle<Palette> _palette;

public:

	ImageFile(OwningHandle<HostImage> image, OwningHandle<Palette> palette)
		: _image(std::move(image)), _palette(std::move(palette))
	{
	}

	~ImageFile()
	{
	}

	const HostImage& getImage() const
	{
		return *_image;
	}

	const Palette& getPalette() const
	{
		return *_palette;
	}

	// Transfers ownership of the image and invalidates the internal handle.
	OwningHandle<HostImage> takeImage()
	{
		return std::move(_image);
	}

	// Transfers ownership of the palette and invalidates the internal handle.
	OwningHandle<Palette> takePalette()
	{
		return std::move(_palette);
	}

};

} // namespace OpenXcom
