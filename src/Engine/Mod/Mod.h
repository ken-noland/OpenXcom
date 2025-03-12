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

#include "ModInfo.h"
#include "../FileSystem/CompositeFileSystem.h"
#include "../Resource/Handle.h"

namespace OpenXcom
{

class EngineContext;
struct ScannedMod;

class Palette;
class DeviceImage;

class Mod
{
protected:

	ModInfo _info;

	std::unique_ptr<CompositeFileSystem> _filesystem;

	// 
	std::unordered_map<std::string, OwningHandle<Palette>> _palettes;
	std::unordered_map<std::string, OwningHandle<DeviceImage>> _images;

public:
	Mod(EngineContext& context, ScannedMod& scannedModInfo);
	Mod(Mod&& other) noexcept;
	~Mod();

	const ModInfo& getInfo() const { return _info; }

	CompositeFileSystem& getFileSystem() { return *_filesystem; }

	void registerPalette(OwningHandle<Palette>&& palette);
	void registerImage(OwningHandle<DeviceImage>&& image);
};

} // namespace OpenXcom
