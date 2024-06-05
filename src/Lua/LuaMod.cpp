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
#include "LuaMod.h"
#include "LuaApi.h"
#include "LuaState.h"
#include "GameScript.h"
#include "../Engine/Logger.h"
#include "../Mod/ModFile.h"

namespace OpenXcom
{

namespace Lua
{

LuaMod::LuaMod(Game& game, const ModFile& modFiles)
	:
	_game(game),
	_modFiles(modFiles)
{
}

LuaMod::~LuaMod()
{
}

bool LuaMod::loadAll()
{
	Log(LOG_INFO) << "Loading Lua...";

	_luaGameScript = std::make_unique<GameScript>(_game);

	// Load all the Lua scripts
	for (const ModInfo* modData : _modFiles.getModData())
	{
		if (modData->hasLua())
		{
			//construct a path to the lua file
			std::filesystem::path luaPath = modData->getPath() / modData->getLuaScript();

			std::unique_ptr<LuaState> luaState = std::make_unique<LuaState>(luaPath, modData, std::initializer_list<LuaApi*>{_luaGameScript.get()});

			//initialize the lua state
			_luaMods.push_back(std::move(luaState));
		}
	}

	Log(LOG_INFO) << "Loading Lua done.";

	return true;
}

} // namespace Lua
} // namespace OpenXcom
