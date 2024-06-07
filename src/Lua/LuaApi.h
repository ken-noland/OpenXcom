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
	int registerTable(lua_State* luaState, const std::string tableName, void* userData = nullptr, int parentIndex = LUA_REGISTRYINDEX, std::function<void(lua_State*, int)> onTableCreated = nullptr);


	/// Helper function to create a function in the table.
	void registerLuaFunction(lua_State* luaState, const std::string functionName, lua_CFunction function);

	/// Helper function to create a class function in the table. This method passes the lua_State to
	/// the member function to allow it to decide what to do with it. KN NOTE: I could probably phase this one out.
	template <typename Owner, int (Owner::*func)(lua_State* luaState)>
	inline void registerClassLuaFunction(lua_State* luaState, const std::string functionName);

	/// Helper function to create a class function in the table. This method automatically extracts the
	/// function parameters and populates it with the correct values.
	template <auto Function>
	inline void registerFunction(lua_State* luaState, const std::string& functionName);

	template <auto Container>
	inline void registerContainer(lua_State* luaState, const std::string& tableName, int parentIndex = LUA_REGISTRYINDEX);

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
	using FunctionType = ReturnType(ClassType::*)(_Args...);

	using ClassMemberFunction = std::true_type;
};

// Partial specialization for function pointers
template <typename _ReturnType, typename... _Args>
struct FunctionTraits<_ReturnType (*)(_Args...)>
{
	using ReturnType = _ReturnType;
	using ArgsTuple = std::tuple<_Args...>;
	using FunctionType = ReturnType(*)(_Args...);

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

	using ClassMemberFunction = std::false_type;
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
void LuaApi::registerFunction(lua_State* luaState, const std::string& functionName)
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


//// Example function to handle table access
//int accessTableItem(lua_State* L)
//{
//	const char* key = luaL_checkstring(L, 2);
//	std::string value = std::string("Value for ") + key;
//	lua_pushstring(L, value.c_str());
//	return 1;
//}


template<typename T>
inline void Testy()
{
	int i = 0;
}

// Static global vector of floats
static std::vector<float> globalVector = {1.1f, 2.2f, 3.3f};

// Templated IteratorState
template <typename Container>
struct IteratorState;

template <typename T>
struct IteratorState<const std::vector<T> &>
{
	using Iterator = typename std::vector<T>::const_iterator;
	Iterator iter;
	Iterator end;
};

template <typename K, typename V>
struct IteratorState<const std::map<K, V> &>
{
	using Iterator = typename std::map<K, V>::const_iterator;
	Iterator iter;
	Iterator end;
};

// Predeclaration of containerIndex function templates
template <typename Container, auto GetContainerFunction>
inline int containerIndex(lua_State* luaState);

template <typename T, auto GetContainerFunction>
inline int containerIndex(lua_State* luaState);

template <typename K, typename V, auto GetContainerFunction>
inline int containerIndex(lua_State* luaState);

// Specialization for std::vector
template <typename T, auto GetContainerFunction>
inline int containerIndex(lua_State* luaState)
{
	const auto& container = GetContainerFunction();
	int index = fromLua<int>(luaState, 2) - 1; // Lua is 1-based, C++ is 0-based
	if (index < 0 || index >= static_cast<int>(container.size()))
	{
		return luaL_error(luaState, "Index out of bounds");
	}
	toLua(luaState, container[index]);
	return 1;
}

// Specialization for std::map
template <typename K, typename V, auto GetContainerFunction>
inline int containerIndex(lua_State* luaState)
{
	const auto& container = GetContainerFunction();
	K key = fromLua<K>(luaState, 2);
	auto it = container.find(key);
	if (it == container.end())
	{
		return luaL_error(luaState, "Key not found");
	}
	toLua(luaState, it->second);
	return 1;
}

// Templated iterator function
template <typename Container, auto GetContainerFunction>
inline int iteratorFunction(lua_State* luaState);

template <typename T, auto GetContainerFunction>
inline int iteratorFunction(lua_State* luaState);

template <typename K, typename V, auto GetContainerFunction>
inline int iteratorFunction(lua_State* luaState);

// Specialization for std::vector
template <typename T, auto GetContainerFunction>
inline int iteratorFunction(lua_State* luaState)
{
	using IterState = IteratorState<T>;
	IterState* state = static_cast<IterState*>(lua_touserdata(luaState, lua_upvalueindex(1)));
	const auto& container = GetContainerFunction();
	if (state->iter != state->end)
	{
		toLua(luaState, std::distance(container.begin(), state->iter) + 1); // Lua index
		toLua(luaState, *state->iter);
		++(state->iter);
		return 2;
	}
	return 0; // No more elements
}

// Specialization for std::map
template <typename K, typename V, auto GetContainerFunction>
inline int iteratorFunction(lua_State* luaState)
{
	using IterState = IteratorState<std::map<K, V> >;
	IterState* state = static_cast<IterState*>(lua_touserdata(luaState, lua_upvalueindex(1)));
	const auto& container = GetContainerFunction();
	if (state->iter != state->end)
	{
		toLua(luaState, state->iter->first);  // Map key
		toLua(luaState, state->iter->second); // Map value
		++(state->iter);
		return 2;
	}
	return 0; // No more elements
}

// Templated pairs function to initialize iteration
template <typename Container, auto GetContainerFunction>
inline int pairsFunction(lua_State* luaState)
{
	using IterState = IteratorState<Container>;
	IterState* state = static_cast<IterState*>(lua_newuserdata(luaState, sizeof(IterState)));
	const Container& container = GetContainerFunction();

	new (state) IterState(); // Placement new for container
	state->iter = container.begin();
	state->end = container.end();

	lua_pushcclosure(luaState, iteratorFunction<Container, GetContainerFunction>, 1);
	lua_pushvalue(luaState, 1); // Push the table as the second value
	lua_pushnil(luaState);      // Initial key value
	return 3;
}


/// Register a container with a lambda that returns the container
template <auto ContainerFunction>
inline void LuaApi::registerContainer(lua_State* luaState, const std::string& tableName, int parentIndex)
{
	auto tempLambda = []() -> const std::vector<float>&
	{
		return globalVector;
	};

	//okay, so to do this, we need to grab the lambda, which will be used to get a reference to the
	// container, then we need to register __index and __pair metamethods which always call the lambda
	// to get the container itself.
	using Container = typename FunctionTraits<decltype(tempLambda)>::ReturnType;

	lua_newtable(luaState); // Create a new table
	lua_newtable(luaState); // Create the metatable

    // Set the __index metamethod to a function that handles access
	lua_CFunction containerIndexPtr = static_cast<lua_CFunction>(containerIndex<Container, tempLambda>);
	lua_pushcfunction(luaState, containerIndexPtr);
	lua_setfield(luaState, -2, "__index");

    // Set the __pairs metamethod to a function that returns keys
	lua_CFunction pairsFunctionPtr = static_cast<lua_CFunction>(pairsFunction<Container, tempLambda>);
	lua_pushcfunction(luaState, pairsFunctionPtr);
	lua_setfield(luaState, -2, "__pairs");

	lua_setmetatable(luaState, -2); // Set the metatable for the new table

	int tableIndex = lua_gettop(luaState);

	lua_pushvalue(luaState, tableIndex);                    // Duplicate the new table
	lua_setfield(luaState, parentIndex, tableName.c_str()); // Set the new table as a field in the parent table
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
inline void LuaApi::registerClassLuaFunction(lua_State* luaState, const std::string functionName)
{
	// Push the function
	lua_pushcfunction(luaState, reinterpret_cast<lua_CFunction>(memberLuaFunctionWrapper<Owner, func>)); // Push the function

	// tableName[functionName] = function
	lua_setfield(luaState, -2, functionName.c_str()); // tableName[functionName] = function
}

} // namespace Lua

} // namespace OpenXcom
