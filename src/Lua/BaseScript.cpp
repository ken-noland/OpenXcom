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
#include "BaseScript.h"

namespace OpenXcom
{

namespace Lua
{

BaseScript::BaseScript() : LuaApi("base")
{
}

BaseScript::~BaseScript()
{
}

template <>
void toLua(lua_State* L, Base* arg)
{
}

template <>
Base* fromLua(lua_State* luaState, int index)
{
	return nullptr;
}



} // namespace Lua

} // namespace OpenXcom
