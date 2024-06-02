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

#include <string>
#include <vector>
#include "../Engine/Logger.h"

extern "C"
{
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

namespace OpenXcom
{

namespace Lua
{

/**
* * LuaApi is the base class for all the LUA API classes. It provides the basic functionality to register the API in the LUA state.
* */
class LuaApi
{
private:
	std::string _name;

protected:
	/// Helper function to create a table. Returns the index of the table on the stack.
	void createTable(lua_State* L, const std::string tableName, int parentIndex = LUA_REGISTRYINDEX);
	void createFunction(lua_State* L, const std::string functionName, lua_CFunction function, void* userData);

public:
	LuaApi(const std::string& name);
	virtual ~LuaApi();

	void registerApi(lua_State* luaState, int parentTableIndex = LUA_REGISTRYINDEX);

	virtual void onRegisterApi(lua_State* luaState, int parentTableIndex){};
};


} // namespace Lua

} // namespace OpenXcom
