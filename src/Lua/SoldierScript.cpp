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
#include "SoldierScript.h"
#include "LuaContainer.h"
#include "LuaProperty.h"
#include "LuaHelpers.h"

#include "../Savegame/Soldier.h"


namespace OpenXcom
{

namespace Lua
{

template <>
void toLua(lua_State* luaState, Soldier* arg)
{
//	int tableIndex = pushTableWithUserdata(luaState, arg);

	int tableIndex = pushTableWithUserdataAndProperties(luaState, arg, [](lua_State* luaState, int index) {
		lua_pushstring(luaState, "name");
		lua_pushstring(luaState, "Soldier 1");
		lua_settable(luaState, index);

		lua_pushstring(luaState, "health");
		lua_pushinteger(luaState, 100);
		lua_settable(luaState, index);

		lua_pushstring(luaState, "morale");
		lua_pushinteger(luaState, -2);
		lua_settable(luaState, index);
	});

	//int top = lua_gettop(luaState);

	//registerProperty<

	//>(luaState, "name", tableIndex);

	//lua_pushstring(luaState, "name");
	//lua_pushstring(luaState, "Soldier 1");
	//lua_settable(luaState, -3);

	//lua_pushstring(luaState, "health");
	//lua_pushinteger(luaState, 100);
	//lua_settable(luaState, -3);

	//lua_pushstring(luaState, "morale");
	//lua_pushinteger(luaState, -2);
	//lua_settable(luaState, -3);

}

template <>
Soldier* fromLua(lua_State* luaState, int index)
{
	// not going to be implemented
	return nullptr;
}

} // namespace Lua

} // namespace OpenXcom
