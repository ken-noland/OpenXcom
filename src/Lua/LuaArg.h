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

	
template <typename T>
inline void toLuaArg(lua_State* L, T arg)
{
	// if you get this error, you need to specialize the toLuaArg function for this type.
	static_assert(sizeof(T) == 0, "No toLuaArg function defined for this type");
}

// KN NOTE: temporarily placing these specializations here, until I find a better place for them
template <>
inline void toLuaArg(lua_State* L, int arg)
{
	lua_pushinteger(L, arg);
}

template <>
inline void toLuaArg(lua_State* L, float arg)
{
	lua_pushnumber(L, static_cast<lua_Number>(arg));
}

template <>
inline void toLuaArg(lua_State* L, double arg)
{
	lua_pushnumber(L, static_cast<lua_Number>(arg));
}

template <>
inline void toLuaArg(lua_State* L, const std::string& arg)
{
	lua_pushstring(L, arg.c_str());
}

template <>
inline void toLuaArg(lua_State* L, const char* arg)
{
	lua_pushstring(L, arg);
}

/// Converts a YAML node to a Lua table
void yamlNodeToLua(lua_State* L, const YAML::Node& node);

template <>
inline void toLuaArg(lua_State* L, const YAML::Node& arg)
{
	yamlNodeToLua(L, arg);
}

template <typename T>
T fromLuaRet(lua_State* L, int index)
{
	// if you get this error, you need to specialize the fromLuaRet function for this type.
	static_assert(sizeof(T) == 0, "No fromLuaRet function defined for this type");
}


} // namespace Lua

} // namespace OpenXcom
