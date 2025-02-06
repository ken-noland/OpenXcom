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
#include "ImageFile.h"
#include "../../Graphics/Image/Image.h"
#include "../../Graphics/Palette/Palette.h"
#include "../../Resource/ResourceManager.h"

namespace OpenXcom
{

ImageFile::ImageFile(OwningHandle<HostImage> image, OwningHandle<Palette> palette)
	: _image(std::move(image)), _palette(std::move(palette))
{
}

ImageFile ::~ImageFile()
{
}

HostImage& ImageFile::getImage()
{
	return *_image;
}

Palette& ImageFile::getPalette()
{
	return *_palette;
}

ResourceHandle<HostImage> ImageFile::getImageHandle()
{
	return _image.getHandle();
}

ResourceHandle<Palette> ImageFile::getPaletteHandle()
{
	return _palette.getHandle();
}

OwningHandle<HostImage> ImageFile::takeImage()
{
	return std::move(_image);
}

OwningHandle<Palette> ImageFile::takePalette()
{
	return std::move(_palette);
}

} // namespace OpenXcom
