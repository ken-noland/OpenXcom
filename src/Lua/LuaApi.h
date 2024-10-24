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
#include <map>
#include <functional>
#include "../Engine/Logger.h"
#include "LuaArg.h"
#include "LuaContainer.h"
#include "LuaFunction.h"

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
	/// Helper function to push a table onto the stack. If userData is provided, it will be stored in the table's metatable.
	int pushTable(lua_State* luaState, void* userData = nullptr);
	void popTable(lua_State* luaState);

	/// Helper function to create a table. Returns the index of the table on the stack. If userData
	/// is provided, it will be stored in the table's metatable. This function pushes the table on
	/// the stack, then calls the onTableCreated function, then pops the table off the stack.
	int registerTable(lua_State* luaState, const std::string tableName, bool pushTable, void* userData = nullptr, int parentIndex = LUA_REGISTRYINDEX, std::function<void(lua_State*, int)> onTableCreated = nullptr);


	/// Helper function to create a function in the table.
	void registerLuaFunction(lua_State* luaState, const std::string functionName, lua_CFunction function);

	/// Helper function to create a class function in the table. This method automatically extracts the
	/// function parameters and populates it with the correct values.
	template <auto Function>
	inline void registerFunction(lua_State* luaState, const std::string& functionName);

public:
	LuaApi(const std::string& name);
	virtual ~LuaApi();

	void registerApi(lua_State* luaState, int parentTableIndex = LUA_REGISTRYINDEX);

	virtual void onRegisterApi(lua_State* luaState, int parentTableIndex){};
};

template <auto Function>
inline void LuaApi::registerFunction(lua_State* luaState, const std::string& functionName)
{
	OpenXcom::Lua::registerFunction<Function>(luaState, functionName);
}

} // namespace Lua

} // namespace OpenXcom
