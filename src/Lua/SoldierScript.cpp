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
	int tableIndex = pushTableWithUserdataAndProperties(luaState, arg, [](lua_State* luaState, int index) {
		lua_pushstring(luaState, "SOME_CONSTANT");
		lua_pushinteger(luaState, 100);
		lua_settable(luaState, index);
	});

	registerProperty<
		[](const Soldier* soldier) -> std::string { return soldier->getName(); },
		[](Soldier* soldier, const std::string& name) { soldier->setName(name); }
	>(luaState, "name", tableIndex, arg);

}

template <>
Soldier* fromLua(lua_State* luaState, int index)
{
	// not going to be implemented
	return nullptr;
}

} // namespace Lua

} // namespace OpenXcom
