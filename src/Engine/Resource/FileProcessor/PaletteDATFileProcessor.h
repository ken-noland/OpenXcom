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
#include <filesystem>
#include <vector>
#include <string>

#include "../Handle.h"
#include "../../Graphics/Palette/Palette.h"
#include "../../Graphics/Palette/PaletteManager.h"

namespace OpenXcom
{

class Palette;
class FileEntry;

struct PaletteLoadParams
{
	// number of colors in the palette that we want to extract
	size_t paletteEntries;

	// number of colors in the palette on file(in some cases, this is different from the number of palette entries)
	size_t filePaletteEntries;
};

class PaletteFile
{
protected:
	std::vector<OwningHandle<Palette>> _palettes;

public:
	PaletteFile() = default;
	PaletteFile(PaletteFile&&) = default;
	~PaletteFile() = default;

	void add(OwningHandle<Palette>& handle) { _palettes.push_back(std::move(handle)); }

	const std::vector<OwningHandle<Palette>>& getPalettes() const { return _palettes; }

	OwningHandle<Palette> takePalette(size_t index)
	{
		OwningHandle<Palette> palette = std::move(_palettes[index]);
		_palettes.erase(_palettes.begin() + index);
		return palette;
	}
};

class PaletteDATFileProcessor
{
protected:
	EngineContext& _context;

	size_t getPaletteBlockFileSize(const PaletteLoadParams& params) const;
	size_t getRequiredFileSize(size_t numPalettes, const PaletteLoadParams& params) const;

public:
	PaletteDATFileProcessor(EngineContext& context);
	~PaletteDATFileProcessor();

	size_t getPaletteCount(const std::unique_ptr<FileEntry>& file, const PaletteLoadParams& params) const;

	PaletteFile load(const std::vector<std::string>& names, const std::unique_ptr<FileEntry>& file, const PaletteLoadParams& params);
};

} // namespace OpenXcom
