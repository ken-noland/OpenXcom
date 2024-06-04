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

extern "C"
{
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

namespace YAML { class Node; }

namespace OpenXcom
{

namespace Lua
{

/// Converts a YAML node to a Lua table
void yamlNodeToLua(lua_State* L, const YAML::Node& node);

/// Converts a Lua table to a YAML node
YAML::Node luaToYamlNode(lua_State* luaState, int index);

template <typename T>
inline void toLua(lua_State* L, T arg)
{
	// if you get this error, you need to specialize the toLuaArg function for this type.
	static_assert(sizeof(T) == 0, "No toLuaArg function defined for this type");
}

template <>
inline void toLua(lua_State* L, int arg)
{
	lua_pushinteger(L, arg);
}

template <>
inline void toLua(lua_State* L, float arg)
{
	lua_pushnumber(L, static_cast<lua_Number>(arg));
}

template <>
inline void toLua(lua_State* L, double arg)
{
	lua_pushnumber(L, static_cast<lua_Number>(arg));
}

template <>
inline void toLua(lua_State* L, const std::string& arg)
{
	lua_pushstring(L, arg.c_str());
}

template <>
inline void toLua(lua_State* L, const char* arg)
{
	lua_pushstring(L, arg);
}

template <>
inline void toLua(lua_State* L, const YAML::Node& arg)
{
	yamlNodeToLua(L, arg);
}

template <typename T>
inline T fromLua(lua_State* L, int index)
{
	// if you get this error, you need to specialize the fromLuaRet function for this type.
	static_assert(sizeof(T) == 0, "No fromLuaRet function defined for this type");
}

template <>
inline int fromLua(lua_State* luaState, int index)
{
	return static_cast<int>(lua_tointeger(luaState, index));
}

template <>
inline float fromLua(lua_State* luaState, int index)
{
	return static_cast<float>(lua_tointeger(luaState, index));
}

template <>
inline double fromLua(lua_State* luaState, int index)
{
	return static_cast<double>(lua_tointeger(luaState, index));
}

template <>
inline std::string fromLua(lua_State* luaState, int index)
{
	const char* str = lua_tostring(luaState, index);
	if (str == nullptr)
	{
		luaL_error(luaState, "Expected a string at index %d", index);
	}
	return std::string(str);
}

template <>
inline const char* fromLua(lua_State* luaState, int index)
{
	return lua_tostring(luaState, index);
}
//
//template <>
//inline YAML::Node fromLua(lua_State* luaState, int index)
//{
//	return luaToYamlNode(luaState, index);
//}

} // namespace Lua

} // namespace OpenXcom
