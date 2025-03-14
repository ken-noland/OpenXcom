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

#include "../../Graphics/Image/Image.h"
#include "../../Graphics/Image/ImageManager.h"

#include "../../Graphics/Palette/Palette.h"
#include "../../Graphics/Palette/PaletteManager.h"

namespace OpenXcom
{

class HostImage;
class Palette;

struct ImageLoadParams
{
	bool loadPalette = false;
};

// a single image file
struct ImageFile
{
	OwningHandle<HostImage> image;
};

// a single image file with a palette
struct ImagePaletteFile
{
	OwningHandle<HostImage> image;
	OwningHandle<Palette> palette;
};

// a set of images
struct ImageSetFile
{
	std::vector<OwningHandle<HostImage>> images;
};

} // namespace OpenXcom
