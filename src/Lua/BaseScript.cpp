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
#include "SoldierScript.h"
#include "LuaHelpers.h"
#include "LuaContainer.h"

#include "../Savegame/Base.h"

namespace OpenXcom
{

namespace Lua
{

template <>
void toLua(lua_State* luaState, Base* arg)
{
	int tableIndex = pushTableWithUserdata(luaState, arg);

	registerContainer<[](Base* base) -> const std::vector<Soldier*>& { return base->getSoldiers(); }>(luaState, "soldiers", tableIndex, arg);
}

template <>
Base* fromLua(lua_State* luaState, int index)
{
	// not going to be implemented
	return nullptr;
}



} // namespace Lua

} // namespace OpenXcom
