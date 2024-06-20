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
#include <tuple>
#include <string>

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
	
template <typename T>
struct FunctionTraits;

// Partial specialization for member function pointers
template <typename _ReturnType, typename _ClassType, typename... _Args>
struct FunctionTraits<_ReturnType (_ClassType::*)(_Args...)>
{
	using ReturnType = _ReturnType;
	using ClassType = _ClassType;
	using ArgsTuple = std::tuple<_Args...>;
	using FunctionType = ReturnType (ClassType::*)(_Args...);

	using ClassMemberFunction = std::true_type;
};

// Partial specialization for function pointers
template <typename _ReturnType, typename... _Args>
struct FunctionTraits<_ReturnType (*)(_Args...)>
{
	using ReturnType = _ReturnType;
	using ArgsTuple = std::tuple<_Args...>;
	using FunctionType = ReturnType (*)(_Args...);

	using ClassMemberFunction = std::false_type;
};

// General case for callables
template <typename F>
struct FunctionTraits : public FunctionTraits<decltype(&F::operator())>
{
};

// Specialization for member function pointers (const) and lambdas
template <typename _ReturnType, typename _ClassType, typename... _Args>
struct FunctionTraits<_ReturnType (_ClassType::*)(_Args...) const>
{
	using ReturnType = _ReturnType;
	using ClassType = _ClassType;
	using ArgsTuple = std::tuple<_Args...>;
	using FunctionType = ReturnType (*)(_Args...);

	using ClassMemberFunction = std::true_type;
};

template <typename Ret, typename... Args>
struct RetArgsTupleToFunction;

template <typename Ret, typename... Args>
struct RetArgsTupleToFunction<Ret, std::tuple<Args...> >
{
	using FunctionSignature = Ret(Args...);

	using ClassMemberFunction = std::false_type;
};

/// Helper function to call member function with variadic arguments
template <typename Ret, typename Owner, typename... Args, std::size_t... I>
Ret callMemberFunction(lua_State* luaState, Ret (Owner::*func)(Args...), Owner* obj, std::index_sequence<I...>)
{
	return (obj->*func)(fromLua<Args>(luaState, I + 2)...);
}

/// Call the member function and return the result using the helper from above
template <typename Ret, typename Owner, typename... Args>
int memberFunctionWrapper(lua_State* luaState, Ret (Owner::*func)(Args...))
{
	// Get the object instance from the table's metatable
	lua_getmetatable(luaState, 1);
	lua_getfield(luaState, -1, "__userdata");
	Owner* ownerPtr = static_cast<Owner*>(lua_touserdata(luaState, -1));
	lua_pop(luaState, 2);

	// Call the member function
	if constexpr (!std::is_void<Ret>::value)
	{
		Ret result = callMemberFunction(luaState, func, ownerPtr, std::index_sequence_for<Args...>{});
		toLua<Ret>(luaState, result);
		return 1;
	}
	else
	{
		callMemberFunction(luaState, func, ownerPtr, std::index_sequence_for<Args...>{});
		return 0;
	}
}

/// Helper function to call C-style function with variadic arguments
template <typename Ret, typename... Args, std::size_t... I>
Ret callCFunction(lua_State* luaState, Ret (*func)(Args...), std::index_sequence<I...>)
{
	return (*func)(fromLua<Args>(luaState, I + 2)...);
}

/// Call the C-style function and return the result using the helper from above
template <typename Ret, typename... Args>
int functionWrapper(lua_State* luaState, Ret (*func)(Args...))
{
	// Call the C-style function
	if constexpr (!std::is_void<Ret>::value)
	{
		Ret result = callCFunction(luaState, func, std::index_sequence_for<Args...>{});
		toLua<Ret>(luaState, result);
		return 1;
	}
	else
	{
		callCFunction(luaState, func, std::index_sequence_for<Args...>{});
		return 0;
	}
}

/// Register function.
template <auto Function>
void registerFunction(lua_State* luaState, const std::string& functionName)
{
	using FunctionDecl = decltype(Function);
	using Traits = FunctionTraits<FunctionDecl>;
	using ReturnType = typename Traits::ReturnType;
	using ClassType = typename Traits::ClassType;

	lua_CFunction luaFunction = nullptr;

	if constexpr (Traits::ClassMemberFunction::value)
	{
		luaFunction = [](lua_State* luaState) -> int
		{
			return memberFunctionWrapper<ReturnType, ClassType>(luaState, Function);
		};
	}
	else
	{
		luaFunction = [](lua_State* luaState) -> int
		{
			return functionWrapper(luaState, (typename Traits::FunctionType)Function);
		};
	}

	// Push the function
	lua_pushcfunction(luaState, reinterpret_cast<lua_CFunction>(luaFunction));

	// tableName[functionName] = function
	lua_setfield(luaState, -2, functionName.c_str());
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
inline void registerClassLuaFunction(lua_State* luaState, const std::string functionName, int tableIndex) // KN NOTE: Maybe move this to LuaFunction.h? I'm not sure it's used anywhere anymore.
{
	// Push the function
	lua_pushcfunction(luaState, reinterpret_cast<lua_CFunction>(memberLuaFunctionWrapper<Owner, func>)); // Push the function

	// tableName[functionName] = function
	lua_setfield(luaState, tableIndex, functionName.c_str()); // tableName[functionName] = function
}


}
}
