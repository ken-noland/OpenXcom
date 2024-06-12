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
#include <yaml-cpp/yaml.h>

namespace OpenXcom
{

namespace Lua
{

template <>
void toLua(lua_State* L, int arg)
{
	lua_pushinteger(L, arg);
}

template <>
void toLua(lua_State* L, float arg)
{
	lua_pushnumber(L, static_cast<lua_Number>(arg));
}

template <>
void toLua(lua_State* L, double arg)
{
	lua_pushnumber(L, static_cast<lua_Number>(arg));
}

template <>
void toLua(lua_State* L, const std::string& arg)
{
	lua_pushstring(L, arg.c_str());
}

template <>
void toLua(lua_State* L, const char* arg)
{
	lua_pushstring(L, arg);
}

template <>
int fromLua(lua_State* luaState, int index)
{
	return static_cast<int>(lua_tointeger(luaState, index));
}

template <>
float fromLua(lua_State* luaState, int index)
{
	return static_cast<float>(lua_tointeger(luaState, index));
}

template <>
double fromLua(lua_State* luaState, int index)
{
	return static_cast<double>(lua_tointeger(luaState, index));
}

template <>
std::string fromLua(lua_State* luaState, int index)
{
	const char* str = lua_tostring(luaState, index);
	if (str == nullptr)
	{
		luaL_error(luaState, "Expected a string at index %d", index);
	}
	return std::string(str);
}

template <>
const char* fromLua(lua_State* luaState, int index)
{
	return lua_tostring(luaState, index);
}


} // namespace Lua

} // namespace OpenXcom