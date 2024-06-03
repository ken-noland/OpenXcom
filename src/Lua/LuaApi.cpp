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

/// Creates a new table and sets the metatable to the userdata. Allows a callback to be called after the table is created, but before it is popped off the stack
int LuaApi::createTable(lua_State* luaState, const std::string tableName, void* userData, int parentIndex, std::function<void(int)> onTableCreated)
{
	lua_newtable(luaState); // Create a new table
	lua_newtable(luaState); // Create the metatable

	// Store the userdata in the metatable
	lua_pushlightuserdata(luaState, userData);
	lua_setfield(luaState, -2, "__userdata");

    lua_setmetatable(luaState, -2); // Set the metatable for the new table

	int tableIndex = lua_gettop(luaState);

	if (parentIndex == LUA_REGISTRYINDEX)
	{
		lua_setglobal(luaState, tableName.c_str()); // Set the table as a global variable
		lua_getglobal(luaState, _name.c_str()); 
	}
	else
	{
		lua_pushvalue(luaState, tableIndex);                    // Duplicate the new table
		lua_setfield(luaState, parentIndex, tableName.c_str()); // Set the new table as a field in the parent table
		//lua_pop(luaState, 1);                                   // Pop the parent table
	}

	//since we are pushing the table on the stack, and we need to clean it up afterwards, then a callback or lambda is needed
	if(onTableCreated)
	{
		onTableCreated(tableIndex);
	}

	lua_pop(luaState, 1);

	return tableIndex;
}

void LuaApi::createFunction(lua_State* L, const std::string functionName, lua_CFunction function)
{
	lua_pushcfunction(L, function);            // Push the function
	lua_setfield(L, -2, functionName.c_str()); // tableName[functionName] = function
}



void LuaApi::registerApi(lua_State* luaState, int parentTableIndex)
{
	// create a new table with the name of the API
	int tableIndex = createTable(luaState, _name, this, parentTableIndex, [this, luaState](int tableIndex) {
		// Register API-specific functions and values
        onRegisterApi(luaState, tableIndex);
	});
}

} // namespace Lua

} // namespace OpenXcom
