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
#include "../../Engine/Surface.h"
#include "PaletteHandle.h"

namespace OpenXcom
{

class Palette;

class PaletteComponent
{
protected:
	// KN TODO: Move the palette from Surface to here
	Surface* _surface;

public:
	PaletteComponent(Surface* surface, PaletteHandle palette);
	~PaletteComponent();

	void setPalette(PaletteHandle palette);
};

/**
 * Container for a system of palettes.
 * Used to manage the palettes in the game.
 */
class PaletteSystem
{
	std::vector<Palette*> _palettes;
	Palette* _backpal;

	/// palette handles use the lower 16 bits for the index and the upper 16 bits for the chunk
	int getPaletteIndex(PaletteHandle handle) const;
	int getPaletteChunk(PaletteHandle handle) const;
	Palette* getPaletteByHandle(PaletteHandle handle) const;

public:
	PaletteSystem();
	~PaletteSystem();

	/// add a new palette to the system
	PaletteHandle addPalette(const std::string& name, const SDL_Color* palette = nullptr, int first = 0, int ncolors = 256);

	/// remove a palette from the system by handle
	void removePalette(PaletteHandle handle);

	[[deprecated("Direct access to Palette should not be allowed")]]
	Palette* getPalette(PaletteHandle handle) const;

	PaletteHandle getPaletteByName(const std::string& name) const;
	PaletteHandle getPaletteByID(const std::string& id, bool alterPal = false) const;

	void setSurfacePalette(Surface* surface, PaletteHandle palette);
};

}
