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

	if (lua_next(luaState, lua_upvalueindex(1)) != 0)
	{
		return 2;                    // Return key and value
	}
	else
	{
		Log(LOG_WARNING) << "Main table exhausted";
	}

	return 0; // No more elements
}


inline int propertiesTablePairs(lua_State* luaState)
{
	luaL_checktype(luaState, 1, LUA_TTABLE);

	lua_pushvalue(luaState, 1); // Push the table as the upvalue for the iterator function

	// Return the iterator function, the table, and the initial key
	lua_pushcclosure(luaState, propertiesTableIterator, 1);
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

/// Registers a property within the metatable of tableIndex. 
template<auto GetterFunction, auto SetterFunction>
inline void registerProperty(lua_State* luaState, const std::string& name, int tableIndex, void* userData)
{
	// Ensure the table at tableIndex has a metatable
	luaL_checktype(luaState, tableIndex, LUA_TTABLE);
	if (lua_getmetatable(luaState, tableIndex) == 0)
	{
		luaL_error(luaState, "Table at index %d has no metatable", tableIndex);
	}

	// Get the __properties table from the metatable
	lua_getfield(luaState, -1, "__properties");
	if (!lua_istable(luaState, -1))
	{
		luaL_error(luaState, "__properties is not a table");
	}

	// Create the property table with getter and setter
	lua_newtable(luaState); // Create the property table
	lua_pushlightuserdata(luaState, (void*)GetterFunction);
	lua_setfield(luaState, -2, "getter");

	if constexpr (SetterFunction != nullptr)
	{
		lua_pushlightuserdata(luaState, (void*)SetterFunction);
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
	lua_pop(luaState, 2); // Pop the __properties table and the parent metatable
}



}
}
