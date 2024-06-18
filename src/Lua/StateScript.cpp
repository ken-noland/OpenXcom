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
#include "StateScript.h"
#include "LuaContainer.h"
#include "LuaHelpers.h"
#include "LuaProperty.h"

#include "../Engine/State.h"
#include "../Engine/Surface.h"

namespace OpenXcom
{

namespace Lua
{

template <>
void toLua(lua_State* luaState, State* arg)
{
	int tableIndex = pushTableWithUserdataAndProperties(luaState, arg, [arg](lua_State* luaState, int tableIndex) {
		registerContainer<&State::getSurfaces>(luaState, "surfaces", tableIndex, arg);
	});

	registerProperty<&State::getName>(luaState, "name", tableIndex, arg);
	registerProperty<&State::getInterfaceCategory>(luaState, "interface_category", tableIndex, arg);
	registerProperty<&State::IsSoundPlayed>(luaState, "is_sound_played", tableIndex, arg);
	registerProperty<&State::IsScreen>(luaState, "is_screen", tableIndex, arg);

}

template <>
State* fromLua(lua_State* luaState, int index)
{
	//not yet implemented
	return nullptr;
}

template <>
void toLua(lua_State* luaState, Surface* arg)
{
	int tableIndex = pushTableWithUserdataAndProperties(luaState, arg);

	registerProperty<&Surface::getX>(luaState, "X", tableIndex, arg);
	registerProperty<&Surface::getY>(luaState, "Y", tableIndex, arg);
	registerProperty<&Surface::getWidth>(luaState, "Width", tableIndex, arg);
	registerProperty<&Surface::getHeight>(luaState, "Height", tableIndex, arg);
}

template <>
Surface* fromLua(lua_State* luaState, int index)
{
	// not yet implemented
	return nullptr;
}

} // namespace Lua

} // namespace OpenXcom
