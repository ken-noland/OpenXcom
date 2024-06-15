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
#include "LuaArg.h"

namespace OpenXcom
{
namespace Lua
{

inline int propertiesTableIndex(lua_State* luaState)
{
	lua_getmetatable(luaState, 1);
	lua_getfield(luaState, -1, "__properties");
	lua_pushvalue(luaState, 2); // Push the key
	lua_gettable(luaState, -2); // Get the property table

	if (lua_istable(luaState, -1))
	{
		lua_getfield(luaState, -1, "getter");
		auto getter = reinterpret_cast<int (*)(lua_State*)>(lua_touserdata(luaState, -1));
		lua_pop(luaState, 1); // Pop the getter lightuserdata
		return getter(luaState);
	}
	else
	{
		lua_pushnil(luaState);
		return 1;
	}
}

inline int propertiesTableNewIndex(lua_State* luaState)
{
	lua_getmetatable(luaState, 1);
	lua_getfield(luaState, -1, "__properties");
	lua_pushvalue(luaState, 2); // Push the key
	lua_gettable(luaState, -2); // Get the property table

	if (lua_istable(luaState, -1))
	{
		lua_getfield(luaState, -1, "setter");
		if (lua_islightuserdata(luaState, -1))
		{
			auto setter = reinterpret_cast<int (*)(lua_State*)>(lua_touserdata(luaState, -1));
			lua_pop(luaState, 1); // Pop the setter lightuserdata
			return setter(luaState);
		}
		else
		{
			const char* fieldName = lua_tostring(luaState, 2);
			lua_pop(luaState, 1); // Pop nil

			luaL_error(luaState, "Attempt to modify a read-only property: %s", fieldName);
		}
	}
	else
	{
		const char* fieldName = lua_tostring(luaState, 2);
		lua_pop(luaState, 1); // Pop nil

		luaL_error(luaState, "Attempt to modify an undefined property: %s", fieldName);
	}
	return 0;
}

inline int propertiesTableIterator(lua_State* luaState)
{
	lua_settop(luaState, 2); // Ensure only two values are on the stack

	int tableIndex = lua_upvalueindex(1);
	if(!lua_istable(luaState, tableIndex))
	{
		return luaL_error(luaState, "Internal error: upvalue1 is not a table");
	}

	int inPropertiesIndex = lua_upvalueindex(2);
	if (!lua_isboolean(luaState, inPropertiesIndex))
	{
		return luaL_error(luaState, "Internal error: upvalue2 is not a boolean");
	}

	bool inProperties = lua_toboolean(luaState, inPropertiesIndex);

	if (!inProperties)
	{
		if (lua_next(luaState, tableIndex) != 0)
		{
			return 2; // Return key and value
		}
		else
		{
			// Switch to properties table
			lua_getmetatable(luaState, tableIndex);
			if (!lua_istable(luaState, -1))
			{
				return luaL_error(luaState, "Internal error: metatable is not a table");
			}

			lua_getfield(luaState, -1, "__properties");
			if (!lua_istable(luaState, -1))
			{
				return luaL_error(luaState, "Internal error: __properties is not a table");
			}

			lua_replace(luaState, tableIndex); // Replace upvalue1 with __properties table
			lua_pushboolean(luaState, 1);
			lua_replace(luaState, inPropertiesIndex); // Set inProperties to true

			lua_settop(luaState, 2); // Reset stack to have exactly two elements
			lua_pushnil(luaState);   // Reset the key for the next iteration
		}
	}

    // Iterating over the __properties table
	if (lua_next(luaState, tableIndex) != 0)
	{
		// Stack now: [..., key, value]
		lua_getfield(luaState, -1, "getter"); // Stack now: [..., key, value, getter]
		if (lua_islightuserdata(luaState, -1))
		{
			auto getter = reinterpret_cast<int (*)(lua_State*, void*)>(lua_touserdata(luaState, -1));

			// Get the metatable of the __properties table
			if(!lua_getmetatable(luaState, -2))   // Stack now: [..., key, value, getter, metatable]
			{
				return luaL_error(luaState, "Internal error: no metatable for __properties table");
			}

			// Get the __userdata from the metatable
			lua_getfield(luaState, -1, "__userdata"); // Stack now: [..., key, value, getter, metatable, __userdata]
			void* userdata = lua_touserdata(luaState, -1);

			// Pop the userdata, metatable, getter, and value
			lua_pop(luaState, 4); // Stack now: [..., key]

			// Call the getter function
			getter(luaState, userdata); // Stack now: [..., key, value]

			return 2; // Return key and value
		}
		else
		{
			// Remove non-function value (getter)
			lua_pop(luaState, 1); // Stack now: [..., key, value]
		}

		return 2;                    // Return key and value
	}

	return 0; // No more elements
}


inline int propertiesTablePairs(lua_State* luaState)
{
	luaL_checktype(luaState, 1, LUA_TTABLE);

    // Push the table to iterate over as upvalue1
	lua_pushvalue(luaState, 1);

	// Push the iteration state (inProperties) as upvalue2
	lua_pushboolean(luaState, 0); // Initially not in properties

	// Return the iterator function, the table, and the initial key
	lua_pushcclosure(luaState, propertiesTableIterator, 2);
	lua_pushvalue(luaState, 1);
	lua_pushnil(luaState); // Push the initial key

	return 3; // Return the iterator function, the table, and the initial key
}


/// Pushes a table with the given userdata and sets up the properties metatable.
inline int pushTableWithUserdataAndProperties(lua_State* luaState, void* userdata, std::function<void(lua_State*, int)> initializer = nullptr)
{
	lua_newtable(luaState); // Create the new table
	int tableIndex = lua_gettop(luaState);

	if (initializer)
	{
		initializer(luaState, tableIndex);
	}

	lua_newtable(luaState); // Create the metatable

	// Store the userdata in the metatable
	lua_pushlightuserdata(luaState, userdata);
	lua_setfield(luaState, -2, "__userdata");

	// Create the __properties table in the metatable
	lua_newtable(luaState);                     // Create the __properties table
	lua_pushvalue(luaState, -1);                // Duplicate the __properties table
	lua_setfield(luaState, -3, "__properties"); // Set the __properties field in the metatable
	lua_pop(luaState, 1);                       // Pop the __properties table

	// Set the __index metamethod
	lua_pushcfunction(luaState, propertiesTableIndex);
	lua_setfield(luaState, -2, "__index");

	// Set the __newindex metamethod
	lua_pushcfunction(luaState, propertiesTableNewIndex);
	lua_setfield(luaState, -2, "__newindex");

	// Set the __pairs metamethod
	lua_pushcfunction(luaState, propertiesTablePairs);
	lua_setfield(luaState, -2, "__pairs");

	lua_setmetatable(luaState, -2); // Set the metatable for the new table

	return tableIndex;
}


template <auto GetterFunction>
inline int callGetter(lua_State* luaState, void* userdata)
{
	using GetterType = decltype(GetterFunction);
	using ReturnType = typename FunctionTraits<GetterType>::ReturnType;
	using ArgsTuple = typename FunctionTraits<GetterType>::ArgsTuple;

	// Call the getter function and push the result to the Lua stack
	if constexpr (std::is_member_function_pointer_v<GetterType>)
	{
		// Member function pointer
		auto object = static_cast<std::remove_pointer_t<std::tuple_element_t<0, ArgsTuple> >*>(userdata);
		ReturnType result = (object->*GetterFunction)();
		toLua(luaState, result);
	}
	else if constexpr (std::tuple_size_v<ArgsTuple> == 1 && std::is_pointer_v<std::tuple_element_t<0, ArgsTuple> >)
	{
		// Free function or lambda with one argument
		ReturnType result = GetterFunction(static_cast<std::remove_pointer_t<std::tuple_element_t<0, ArgsTuple> >*>(userdata));
		toLua<ReturnType>(luaState, result);
	}
	else
	{
		static_assert(std::is_invocable_v<GetterType>, "Unsupported getter function type");
	}

	return 1; // Return the number of results
}

template <auto SetterFunction>
inline int callSetter(lua_State* luaState, void* userdata)
{
	using SetterType = decltype(SetterFunction);
	using ArgsTuple = typename FunctionTraits<SetterType>::ArgsTuple;

	// Call the setter function with the value from the Lua stack
	if constexpr (std::is_member_function_pointer_v<SetterType>)
	{
		// Member function pointer
		auto object = static_cast<std::remove_pointer_t<std::tuple_element_t<0, ArgsTuple> >*>(userdata);
		using ArgType = std::remove_cvref_t<std::tuple_element_t<1, ArgsTuple> >;
		ArgType value = fromLua<ArgType>(luaState, 1);
		(object->*SetterFunction)(value);
	}
	else if constexpr (std::tuple_size_v<ArgsTuple> == 2 && std::is_pointer_v<std::tuple_element_t<0, ArgsTuple> >)
	{
		// Free function or lambda with two arguments
		using ArgType = std::remove_cvref_t<std::tuple_element_t<1, ArgsTuple> >;
		ArgType value = fromLua<ArgType>(luaState, 1);
		SetterFunction(static_cast<std::remove_pointer_t<std::tuple_element_t<0, ArgsTuple> >*>(userdata), value);
	}
	else
	{
		static_assert(std::is_invocable_v<SetterType>, "Unsupported setter function type");
	}

	return 0; // Setters don't return any values
}

/// Registers a property within the metatable of tableIndex. 
template<auto GetterFunction, auto SetterFunction = nullptr, typename UserdataType>
inline void registerProperty(lua_State* luaState, const std::string& name, int tableIndex, UserdataType* userData)
{
	// Ensure the table at tableIndex has a metatable
	luaL_checktype(luaState, tableIndex, LUA_TTABLE);
	if (lua_getmetatable(luaState, tableIndex) == 0)
	{
		luaL_error(luaState, "Table at index %d has no metatable", tableIndex);
	}	// stack looks like: [.., metatable]

	// Get the __properties table from the metatable
	lua_getfield(luaState, -1, "__properties");
	if (!lua_istable(luaState, -1))
	{
		luaL_error(luaState, "__properties is not a table");
	}	// stack looks like: [.., metatable, __properties]

	// Create the property table with getter and setter
	lua_newtable(luaState); // Create the property table
	lua_pushlightuserdata(luaState, (void*)callGetter<GetterFunction>);
	lua_setfield(luaState, -2, "getter");

	if constexpr (SetterFunction != nullptr)
	{
		lua_pushlightuserdata(luaState, (void*)callSetter<SetterFunction>);
		lua_setfield(luaState, -2, "setter");
	}

	// Set the metatable for the property table to store userdata
	lua_newtable(luaState); // Create a new metatable for the property table
	lua_pushlightuserdata(luaState, userData);
	lua_setfield(luaState, -2, "__userdata");
	lua_setmetatable(luaState, -2); // Set the metatable for the property table

	// Add the property table to the __properties table
	lua_setfield(luaState, -2, name.c_str());

	// Clean up the stack
	lua_pop(luaState, 2); // stack looks like: [..]
}



}
}
