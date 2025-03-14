#include "Mod.h"
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
#include "Mod.h"
#include "ModScanner.h"
#include "../../version.h"
#include "../Logger.h"

#include "Loader/Current/ModLoader.h"
#include "Loader/8.1.2/ModLoader.h"

#include "../Graphics/Palette/Palette.h"
#include "../Graphics/Palette/PaletteManager.h"
#include "../Graphics/Image/Image.h"
#include "../Graphics/Image/ImageSet.h"
#include "../Graphics/Image/ImageManager.h"

namespace OpenXcom
{

Mod::Mod(EngineContext& context, ScannedMod& scannedModInfo)
	: _info(scannedModInfo.info),						// copy the ModInfo
	  _filesystem(std::move(scannedModInfo.filesystem))	// move the filesystem
{
	// check the loader version
	if (_info.loaderVersion >= semver::version(OPENXCOM_VERSION_MAJOR, OPENXCOM_VERSION_MINOR, OPENXCOM_VERSION_PATCH))
	{
		if (!ModLoader_Current::load(this))
		{
			std::ostringstream os;
			os << "Failed to load mod '" << _info.id << "' with current loader version";
			throw std::runtime_error(os.str());
		}
	}
	else if (_info.loaderVersion == semver::version(8, 1, 2))
	{
		if (!ModLoader_8_1_2::load(context, this))
		{
			std::ostringstream os;
			os << "Failed to load mod '" << _info.id << "' with loader version 8.1.2.";
			throw std::runtime_error(os.str());
		}
	}
	else
	{
		std::ostringstream os;
		os << "Mod '" << _info.id << "' has unsupported loader version.";
		throw std::runtime_error(os.str());
	}
}

Mod::Mod(Mod&& other) noexcept
{
	_info = std::move(other._info);
	_filesystem = std::move(other._filesystem);

	_palettes = std::move(other._palettes);
	_images = std::move(other._images);
}

Mod::~Mod()
{
}

void Mod::registerPalette(OwningHandle<Palette>&& palette)
{
	std::string name = palette->name();
	_palettes[name] = std::move(palette);

	Log(LOG_DEBUG) << "Registered palette '" << name << "' for mod '" << _info.id << "'";
}


void Mod::registerImage(OwningHandle<DeviceImage>&& image)
{
	std::string name = image->name();
	_images[name] = std::move(image);

	Log(LOG_DEBUG) << "Registered image '" << name << "' for mod '" << _info.id << "'";
}

void Mod::registerImageSet(ImageSet&& images)
{
	std::string name = images.name();
	_imageSets[name] = std::move(images);

	Log(LOG_DEBUG) << "Registered image set '" << name << "' for mod '" << _info.id << "'";
}

} // namespace OpenXcom
