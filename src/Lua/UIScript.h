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

#include "LuaApi.h"
#include "LuaDispatch.h"

namespace OpenXcom
{

class Game;

namespace Lua
{

class UIScript : public LuaApi
{
private:
	Game& _game;

	//this is just a test function which is triggered on a certain keypress, which allows us to output the state of the game at any given time.
	using LuaOnTestEvent = LuaSimpleCallback<void>;
	LuaOnTestEvent _onTestEvent;

public:
	UIScript(Game& game);
	~UIScript();

	/// Registers the game API with the Lua state.
	virtual void onRegisterApi(lua_State* luaState, int parentTableIndex) override;

	LuaOnTestEvent& getOnTestEvent() { return _onTestEvent; }
};

} // namespace Lua
} // namespace OpenXcom
