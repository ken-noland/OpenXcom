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

#include "ResourceSystem.h"

#include "Font/FontManager.h"
#include "Palette/PaletteManager.h"

namespace OpenXcom
{

ResourceSystem::ResourceSystem()
{
	_fontManager = std::make_unique<FontManager>();
	_paletteManager = std::make_unique<PaletteManager>();
}

ResourceSystem::~ResourceSystem()
{
}

FontManager& ResourceSystem::getFontManager()
{
	return *_fontManager;
}

PaletteManager& ResourceSystem::getPaletteManager()
{
	return *_paletteManager;
}

} // namespace OpenXcom
