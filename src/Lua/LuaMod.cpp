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
#include "GameScript.h"
#include "../Engine/Logger.h"

namespace OpenXcom
{

namespace Lua
{

LuaMod::LuaMod(Game* game)
	:
	_game(game)
{
}

LuaMod::~LuaMod()
{
}

bool LuaMod::loadAll()
{
	Log(LOG_INFO) << "Loading Lua...";

	_luaGameScript = std::make_unique<Lua::GameScript>();

	//for (const ModInfo* modData : _luaMods)
	//{
	//	// okay, so this gets a bit tricky. The whole "mod" part was originally developed just to allow cascading rulesets, so
	//	//  there is no central "mod" object that I can utilize for the LuaState object. What this means is that I have to
	//	//  manage the life-cycle of the Lua stuff separately from everything else.
	//	if (modData.info->hasLua())
	//	{
	//		std::filesystem::path luaPath = modData.info->getPath() / modData.info->getLuaScript();
	//		_luaMods.push_back(Lua::LuaState(luaPath, &modData));
	//	}
	//}
	Log(LOG_INFO) << "Loading Lua done.";


	return true;
}

} // namespace Lua
} // namespace OpenXcom
