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
#include "LuaHelpers.h"

extern "C"
{
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

int OpenXcom::Lua::pushTableWithUserdata(lua_State* luaState, void* userData)
{
	lua_newtable(luaState); // Create a new table
	lua_newtable(luaState); // Create the metatable

	if (userData != nullptr)
	{
		// Store the userdata in the metatable
		lua_pushlightuserdata(luaState, userData);
		lua_setfield(luaState, -2, "__userdata");
	}

	lua_setmetatable(luaState, -2); // Set the metatable for the new table

	return lua_gettop(luaState);
}
