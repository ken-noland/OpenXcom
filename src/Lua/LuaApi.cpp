#include "LuaApi.h"
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
#include "LuaApi.h"


namespace OpenXcom
{

namespace Lua
{


LuaApi::LuaApi(const std::string& name)
	:
	_name(name)
{
}

LuaApi::~LuaApi()
{
}

void LuaApi::createTable(lua_State* L, const std::string tableName, int parentIndex)
{
	lua_newtable(L); // Create a new table

	if (parentIndex == LUA_REGISTRYINDEX)
	{
		lua_setglobal(L, tableName.c_str()); // Set the table as a global variable
	}
	else
	{
		lua_pushvalue(L, -1);                            // Duplicate the new table
		lua_setfield(L, parentIndex, tableName.c_str()); // Set the new table as a field in the parent table
	}
}

void LuaApi::createFunction(lua_State* L, const std::string functionName, lua_CFunction function, void* userData)
{
}

void LuaApi::registerApi(lua_State* luaState, int parentTableIndex)
{
	// create a new table with the name of the API
	createTable(luaState, _name, parentTableIndex);

	// Push table to the stack
	lua_getglobal(luaState, _name.c_str()); 

	onRegisterApi(luaState, lua_gettop(luaState));

	// Pop table from the stack
	lua_pop(luaState, 1);
}

} // namespace Lua

} // namespace OpenXcom
