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
#include "LuaHelpers.h"
#include "LuaProperty.h"

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

int LuaApi::pushTable(lua_State* luaState, void* userData)
{
	return pushTableWithUserdata(luaState, userData);
}

void LuaApi::popTable(lua_State* luaState)
{
	lua_pop(luaState, 1);
}

/// Creates a new table and sets the metatable to the userdata. Allows a callback to be called after the table is created, but before it is popped off the stack
int LuaApi::registerTable(lua_State* luaState, const std::string tableName, bool shouldPushTable, void* userData, int parentIndex, std::function<void(lua_State*, int)> onTableCreated)
{
	int tableIndex = pushTable(luaState, userData);



	//since we are pushing the table on the stack, and we need to clean it up afterwards, then a callback or lambda is needed
	if(onTableCreated)
	{
		onTableCreated(luaState, tableIndex);
	}

	if (!shouldPushTable)
	{
		popTable(luaState);
	}

	return tableIndex;
}

void LuaApi::registerLuaFunction(lua_State* luaState, const std::string functionName, lua_CFunction function)
{
	lua_pushcfunction(luaState, function);     // Push the function
	lua_setfield(luaState, -2, functionName.c_str()); // tableName[functionName] = function
}



void LuaApi::registerApi(lua_State* luaState, int parentTableIndex)
{
	// create a new table with the name of the API
//	int tableIndex = registerTable(luaState, _name, false, this, parentTableIndex, 
	int tableIndex = pushTableWithUserdataAndProperties(luaState, this, [this](lua_State* luaState, int tableIndex) {
		// Register API-specific functions and values
		onRegisterApi(luaState, tableIndex);
	}); // Stack now: [..., table]

	if (parentTableIndex == LUA_REGISTRYINDEX)
	{
		// Set the table as a global variable
		lua_setglobal(luaState, _name.c_str());                  // Stack now: [...]
	}
	else
	{
		// set the table as a field in the parent table
		lua_setfield(luaState, parentTableIndex, _name.c_str()); // Stack now: [..., table]
	}
}

} // namespace Lua

} // namespace OpenXcom
