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
#include <functional>
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
	int createTable(lua_State* luaState, const std::string tableName, void* userData = nullptr, int parentIndex = LUA_REGISTRYINDEX, std::function<void(int)> onTableCreated = nullptr);

	/// Helper function to create a function in the table.
	void createFunction(lua_State* luaState, const std::string functionName, lua_CFunction function);

	/// Helper function to create a class function in the table. This method passes the lua_State to
	//  the member function to allow it to decide what to do with it.
	template <typename Owner, int (Owner::*func)(lua_State* luaState)>
	inline void createClassLuaFunction(lua_State* luaState, const std::string functionName);

	/// Helper function to create a class function in the table. This method automatically extracts the
	/// function parameters and populates it with the correct values.
	template <auto Function>
	inline void createClassFunction(lua_State* L, const std::string& functionName);

public:
	LuaApi(const std::string& name);
	virtual ~LuaApi();

	void registerApi(lua_State* luaState, int parentTableIndex = LUA_REGISTRYINDEX);

	virtual void onRegisterApi(lua_State* luaState, int parentTableIndex){};
};

template <typename T>
struct FunctionTraits;

// Partial specialization for member function pointers
template <typename _ReturnType, typename _ClassType, typename... _Args>
struct FunctionTraits<_ReturnType (_ClassType::*)(_Args...)>
{
	using ReturnType = _ReturnType;
	using ClassType = _ClassType;
	using ArgsTuple = std::tuple<_Args...>;
	using MemberFunctionPointer = ReturnType(ClassType::*)(_Args...);
};

// Helper function to call member function with variadic arguments
template <typename Ret, typename Owner, typename... Args, std::size_t... I>
Ret callMemberFunction(lua_State* luaState, Ret (Owner::*func)(Args...), Owner* obj, std::index_sequence<I...>)
{
	return (obj->*func)(fromLua<Args>(luaState, I + 2)...);
}

template <typename Ret, typename Owner, typename... Args>
int memberFunctionWrapper(lua_State* luaState, Ret (Owner::*func)(Args...))
{
	// Get the object instance from the table's metatable
	lua_getmetatable(luaState, 1);
	lua_getfield(luaState, -1, "__userdata");
	Owner* ownerPtr = static_cast<Owner*>(lua_touserdata(luaState, -1));
	lua_pop(luaState, 2);

	//// Call the member function
	if constexpr (!std::is_void<Ret>::value)
	{
		Ret result = callMemberFunction(luaState, func, ownerPtr, std::index_sequence_for<Args...>{});
		return 1;
	} else {
		callMemberFunction(luaState, func, ownerPtr, std::index_sequence_for<Args...>{});
		return 0;
	}
}

template <auto Function>
void LuaApi::createClassFunction(lua_State* luaState, const std::string& functionName)
{
	using FunctionDecl = decltype(Function);
	using Traits = FunctionTraits<FunctionDecl>;
	using ReturnType = typename Traits::ReturnType;
	using ClassType = typename Traits::ClassType;

	// Define the member function wrapper
	lua_CFunction memberFunction = [](lua_State* luaState) -> int
	{
		return memberFunctionWrapper<ReturnType, ClassType>(luaState, Function);
	};

	lua_pushcfunction(luaState, reinterpret_cast<lua_CFunction>(memberFunction)); // Push the function
	lua_setfield(luaState, -2, functionName.c_str());                                                    // tableName[functionName] = function
}


/// Static function to act as a trampoline for member functions
template <typename Owner, int (Owner::*func)(lua_State*)>
int memberLuaFunctionWrapper(lua_State* luaState)
{
	// Get the object instance from the table's metatable
	lua_getmetatable(luaState, 1);
	lua_getfield(luaState, -1, "__userdata");
	void* userdata = lua_touserdata(luaState, -1);
	lua_pop(luaState, 2);

	// Call the member function
	Owner* obj = static_cast<Owner*>(userdata);
	return (obj->*func)(luaState);
}

/// Helper function to create a class function in the table.
template <typename Owner, int (Owner::*func)(lua_State*)>
inline void LuaApi::createClassLuaFunction(lua_State* luaState, const std::string functionName)
{
	lua_pushcfunction(luaState, reinterpret_cast<lua_CFunction>(memberLuaFunctionWrapper<Owner, func>)); // Push the function
	lua_setfield(luaState, -2, functionName.c_str());                // tableName[functionName] = function
}

} // namespace Lua

} // namespace OpenXcom
