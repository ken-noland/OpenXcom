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

namespace OpenXcom
{

namespace Lua
{

template <typename T>
void toLua(lua_State* L, T arg)
{
	// if you get this error, you need to specialize the toLua function for this type.
	static_assert(sizeof(T) == 0, "No toLua function defined for this type");
}

template <> void toLua(lua_State* L, int arg);
template <> void toLua(lua_State* L, float arg);
template <> void toLua(lua_State* L, double arg);
template <> void toLua(lua_State* L, const std::string& arg);
template <> void toLua(lua_State* L, std::string arg);
template <> void toLua(lua_State* L, const char* arg);
template <> void toLua(lua_State* L, size_t arg);
template <> void toLua(lua_State* L, __int64 arg);


template <typename T>
inline T fromLua(lua_State* L, int index)
{
	// if you get this error, you need to specialize the fromLuaRet function for this type.
	static_assert(sizeof(T) == 0, "No fromLuaRet function defined for this type");
}

template <> int fromLua(lua_State* luaState, int index);
template <> float fromLua(lua_State* luaState, int index);
template <> double fromLua(lua_State* luaState, int index);
template <> std::string fromLua(lua_State* luaState, int index);
template <> const char* fromLua(lua_State* luaState, int index);
template <> size_t fromLua(lua_State* luaState, int index);
template <> __int64 fromLua(lua_State* luaState, int index);

} // namespace Lua

} // namespace OpenXcom
