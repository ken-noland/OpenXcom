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
#include "GameScript.h"
#include "../Engine/Game.h"

namespace OpenXcom
{

namespace Lua
{

GameScript::GameScript(Game& game)
	:
	LuaApi("game"),
	_game(game),
	_onTest("on_test_event"),
	_onLoadGame("on_load_game")
{
}

GameScript::~GameScript()
{
}

void GameScript::onRegisterApi(lua_State* luaState, int parentTableIndex)
{
	_geoscapeScript.registerApi(luaState, parentTableIndex);
	_onTest.registerApi(luaState, parentTableIndex);
}


} // namespace Lua

} // namespace OpenXcom
