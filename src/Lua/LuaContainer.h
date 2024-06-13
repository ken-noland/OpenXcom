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

#include "LuaFunction.h"

namespace OpenXcom
{
namespace Lua
{


// Templated IteratorState
template <typename Container>
struct IteratorState;

template <typename T>
struct IteratorState<const std::vector<T>&>
{
	using Iterator = typename std::vector<T>::const_iterator;
	Iterator iter;
	Iterator end;
};

template <typename K, typename V>
struct IteratorState<const std::map<K, V>&>
{
	using Iterator = typename std::map<K, V>::const_iterator;
	Iterator iter;
	Iterator end;
};

template <typename Test>
void Testy()
{
}

template <typename Traits, auto GetContainerFunction>
inline typename Traits::ReturnType getContainer(lua_State* luaState, int index = 1)
{
	using ArgsTuple = typename Traits::ArgsTuple;
	if constexpr (std::tuple_size_v<ArgsTuple> == 1)
	{
		if constexpr (std::is_pointer_v<std::tuple_element_t<0, ArgsTuple> >)
		{
			using InstanceType = std::remove_pointer_t<std::tuple_element_t<0, typename Traits::ArgsTuple> >;

			// Get the metatable of the table at the given index
			if (!lua_getmetatable(luaState, index))
			{
				throw new std::exception("Error: No metatable found at the given index.");
			}

			// Get the __userdata field from the metatable
			lua_getfield(luaState, -1, "__userdata");

			if (!lua_islightuserdata(luaState, -1))
			{
				lua_pop(luaState, 2); // Pop the metatable and the invalid field
				throw new std::exception("Error: __userdata field is not a userdata type.");
			}

			// Get the userdata
			InstanceType* instance = static_cast<InstanceType*>(lua_touserdata(luaState, -1));
			if(instance == nullptr)
			{
				//instance is nullptr here
			}

			// Pop the metatable and userdata from the stack
			lua_pop(luaState, 2);

			return GetContainerFunction(instance);
		}
		else
		{
			static_assert(!std::is_pointer_v<std::tuple_element_t<0, ArgsTuple> >, "First parameter must be a pointer type");
		}
	}
	else
	{
		return GetContainerFunction();
	}
}


// containerIndex function template
template <typename Traits, auto GetContainerFunction, auto ContainerEnabledFunction>
inline int containerIndex(lua_State* luaState)
{
	if constexpr (ContainerEnabledFunction != nullptr)
	{
		if (!ContainerEnabledFunction())
		{
			lua_pushnil(luaState);
			return 1;
		}
	}

	//
	const auto& container = getContainer<Traits, GetContainerFunction>(luaState);
	int index = fromLua<int>(luaState, 2) - 1; // Lua is 1-based, C++ is 0-based
	if (index < 0 || index >= static_cast<int>(container.size()))
	{
		return luaL_error(luaState, "Index out of bounds");
	}
	toLua(luaState, container[index]);
	return 1;
}

// Templated iterator function
template <typename Traits, auto GetContainerFunction>
inline int iteratorFunction(lua_State* luaState)
{
	using Container = typename Traits::ReturnType;
	using IterState = IteratorState<Container>;
	IterState* state = static_cast<IterState*>(lua_touserdata(luaState, lua_upvalueindex(1)));
	const auto& container = getContainer<Traits, GetContainerFunction>(luaState);
	if (state->iter != state->end)
	{
		toLua(luaState, std::distance(container.begin(), state->iter) + 1); // Lua index
		toLua(luaState, *state->iter);
		++(state->iter);
		return 2;
	}
	return 0; // No more elements
}

inline int emptyIteratorFunction(lua_State* luaState)
{
	return 0;
}

// Templated pairs function to initialize iteration
template <typename Traits, auto GetContainerFunction, auto ContainerEnabledFunction = nullptr>
inline int pairsFunction(lua_State* luaState)
{
	using Container = typename Traits::ReturnType;
	using IterState = IteratorState<Container>;

	if constexpr (ContainerEnabledFunction != nullptr)
	{
		if (!ContainerEnabledFunction())
		{
			lua_pushcclosure(luaState, emptyIteratorFunction, 1); // Iterator function
			lua_pushvalue(luaState, 1);                           // Table
			lua_pushnil(luaState);                                // Initial key value
			return 3;
		}
	}

	IterState* state = static_cast<IterState*>(lua_newuserdata(luaState, sizeof(IterState)));

	const Container& container = getContainer<Traits, GetContainerFunction>(luaState, 1);

	new (state) IterState(); // Placement new for container
	state->iter = container.begin();
	state->end = container.end();

	lua_pushcclosure(luaState, iteratorFunction<Traits, GetContainerFunction>, 1);
	lua_pushvalue(luaState, 1); // Push the table as the second value
	lua_pushnil(luaState);      // Initial key value
	return 3;
}

/// Register a container with a lambda that returns the container
template <auto GetContainerFunction, auto ContainerEnabledFunction = nullptr>
inline void registerContainer(lua_State* luaState, const std::string& tableName, int parentIndex, void* userData)
{
	// okay, so to do this, we need to grab the lambda, which will be used to get a reference to the
	//  container, then we need to register __index and __pair metamethods which always call the lambda
	//  to get the container itself.
	using Traits = FunctionTraits<decltype(GetContainerFunction)>;
	using Container = typename Traits::ReturnType;

	lua_newtable(luaState); // Create a new table
	lua_newtable(luaState); // Create the metatable

	// Set the __index metamethod to a function that handles access
	lua_CFunction containerIndexPtr = static_cast<lua_CFunction>(containerIndex<Traits, GetContainerFunction, ContainerEnabledFunction>);
	lua_pushcfunction(luaState, containerIndexPtr);
	lua_setfield(luaState, -2, "__index");

	// Set the __pairs metamethod to a function that returns keys
	lua_CFunction pairsFunctionPtr = static_cast<lua_CFunction>(pairsFunction<Traits, GetContainerFunction, ContainerEnabledFunction>);
	lua_pushcfunction(luaState, pairsFunctionPtr);
	lua_setfield(luaState, -2, "__pairs");

	// Store the userdata in the metatable
	lua_pushlightuserdata(luaState, userData);
	lua_setfield(luaState, -2, "__userdata");

	lua_setmetatable(luaState, -2); // Set the metatable for the new table

	int tableIndex = lua_gettop(luaState);

	lua_pushvalue(luaState, tableIndex);                    // Duplicate the new table
	lua_setfield(luaState, parentIndex, tableName.c_str()); // Set the new table as a field in the parent table
		
    // Ensure stack balance
	lua_pop(luaState, 1); // Pop the duplicated table
}



}
}
