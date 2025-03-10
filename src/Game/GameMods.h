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
#include "../Engine/Mod/ModScanner.h"
#include "../Engine/Mod/Mod.h"

namespace OpenXcom
{

class GameContext;

// helper struct to get some context when canActivateMod or activateMod is called
struct ActivateModResult
{
	enum class ResultType
	{
		Success,
		AlreadyActive,
		Failed_CouldNotLocate,
		Failed_MissingDependencies,
		Failed_Conflicts
	};

	ResultType result;
	std::string failReason; // or empty if succeeded

	// Dependencies that couldn't be satisfied
	// (nonexistent, conflicting, etc.).
	std::vector<std::string> errorDeps;
};


class GameMods
{
protected:
	GameContext& _context;

	// little struct to keep track of mods that have been scanned and activated
	struct ScannedMods
	{
		std::vector<ScannedMod> masters;
		std::vector<ScannedMod> activeMods;
		std::vector<ScannedMod> inactiveMods;

		ScannedMod* activeMaster = nullptr;
	};

	ScannedMods _scannedMods;

	// the mods that have been loaded
	struct LoadedMods
	{
		std::vector<Mod> mods;
		Mod* activeMaster;
	};

	LoadedMods _loadedMods;

	bool setupScanner(ModScanner& scanner);

public:
	GameMods(GameContext& context);
	~GameMods();

	bool load();

	// note: this moves the mod to the active list, but does not load its resources.
	ActivateModResult canActivateMod(const std::string& id) const;
	ActivateModResult activateMod(const std::string& id);

	bool activateMaster(const std::string& id);
};

} // namespace OpenXcom
