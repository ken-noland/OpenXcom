#include "Palette.h"
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
#include "Palette.h"
#include "../../Engine/Palette.h"

#include "../../Engine/Game.h"
#include "../../Mod/Mod.h"
#include "../../Mod/RuleInterface.h"


#include <simplerttr.h>
SIMPLERTTR
{
	SimpleRTTR::Registration().Type<OpenXcom::PaletteComponent>();
}


namespace OpenXcom
{

PaletteComponent::PaletteComponent(Surface* surface, PaletteHandle palette) : _surface(surface)
{
	setPalette(palette);
}

PaletteComponent::~PaletteComponent()
{
}

void PaletteComponent::setPalette(PaletteHandle palette)
{
	//KN Note: This function is temporary until we move Surface to it's own system
	PaletteSystem& paletteSystem = getSystem<PaletteSystem>();
	paletteSystem.setSurfacePalette(_surface, palette);
}

PaletteSystem::PaletteSystem()
	: _backpal(nullptr)
{
}

PaletteSystem::~PaletteSystem()
{
	for (Palette* palette : _palettes)
	{
		if (palette != nullptr)
		{
			delete palette;
		}
	}
}


int PaletteSystem::getPaletteIndex(PaletteHandle handle) const
{
	return (int)handle & 0xFFFF;
}

int PaletteSystem::getPaletteChunk(PaletteHandle handle) const
{
	return ((int)handle >> 16) - 1;
}

Palette* PaletteSystem::getPaletteByHandle(PaletteHandle handle) const
{
	int index = getPaletteIndex(handle);
	if (index < _palettes.size())
	{
		return _palettes[index];
	}
	return nullptr;
}

PaletteHandle PaletteSystem::addPalette(const std::string& name, const SDL_Color* palette, int first, int ncolors)
{
	Palette* newPalette = new Palette(name, palette, first, ncolors);
	_palettes.push_back(newPalette);
	return (PaletteHandle)(_palettes.size() - 1);
}

void PaletteSystem::removePalette(PaletteHandle handle)
{
	// It's worth noting that this will continue to grow the list if you keep added new
	// palettes. This isn't much of a concern for most use cases, so leaving empty slots
	// in the palette list is fine.

	int index = getPaletteIndex(handle);
	Palette* palette = getPaletteByHandle(handle);
	if (palette != nullptr)
	{
		delete palette;
		_palettes[index] = nullptr;
	}
}

[[deprecated("Direct access to Palette should not be allowed")]]
Palette* PaletteSystem::getPalette(PaletteHandle handle) const
{
	return getPaletteByHandle(handle);
}

PaletteHandle PaletteSystem::getPaletteByName(const std::string& name) const
{
	for (size_t i = 0; i < _palettes.size(); i++)
	{
		if (_palettes[i]->getName() == name)
		{
			return (PaletteHandle)i;
		}
	}
	return PaletteHandle::Invalid;
}

PaletteHandle PaletteSystem::getPaletteByID(const std::string& id, bool alterPal) const
{
	//copy of the old State::setInterface
	int paletteBlock = -1;
	std::string paletteName;

	// Until we have a proper palette system, we need to lean on the mod interface.
	RuleInterface* ruleInterface = getGame()->getMod()->getInterface(id);
	if (ruleInterface)
	{
		RuleInterface* ruleInterfaceParent = getGame()->getMod()->getInterface(ruleInterface->getParent());
		paletteName = ruleInterface->getPalette();

		Element* element = ruleInterface->getElement("palette");

		if (ruleInterfaceParent)
		{
			if (!element)
			{
				element = ruleInterfaceParent->getElement("palette");
			}
			if (paletteName.empty())
			{
				paletteName = ruleInterfaceParent->getPalette();
			}
		}
		if (element)
		{
			int block = alterPal ? element->color2 : element->color;
			if (block != INT_MAX)
			{
				paletteBlock = block;
			}
		}
	}

	if (paletteName.empty())
	{
		paletteName = "PAL_GEOSCAPE";
	}

	PaletteHandle handle = getPaletteByName(paletteName);

	if (paletteBlock != -1 && handle != PaletteHandle::Invalid)
	{
		return (PaletteHandle)(((paletteBlock + 1) << 16) + (uint32_t)getPaletteByName(paletteName));
	}

	return handle;
}

void PaletteSystem::setSurfacePalette(Surface* surface, PaletteHandle paletteHandle)
{
	if (paletteHandle == PaletteHandle::Invalid)
	{
		return;
	}

	Palette* pal = getPaletteByHandle(paletteHandle);
	if (pal != nullptr)
	{
		SDL_SetColors(surface->getSDLSurface(), pal->getColors(), 0, pal->getColorCount());
	}

	// KN NOTE: All of this is only used (as far as I can tell) for background images. It would
	// make more sense to move this logic to the background component, but I'm also trying to keep
	// as close to the original source as possible.

	// background images need a separate palette to override the main palette, so that is done here
	int chunk = getPaletteChunk(paletteHandle);
	if (chunk >= 0)
	{
		if (_backpal == nullptr)
		{
			_backpal = getPalette(getPaletteByName("BACKPALS.DAT"));
			if (_backpal == nullptr)
			{
				// no backpal, so just return
				return;
			}
		}

		SDL_SetColors(surface->getSDLSurface(), _backpal->getColors() + Palette::blockOffset(chunk), Palette::backPos, 16);
	}
}

}

