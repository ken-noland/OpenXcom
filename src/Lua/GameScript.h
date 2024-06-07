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
#include "BaseScript.h"
#include "GeoscapeScript.h"
#include "BattlescapeScript.h"
#include "BasescapeScript.h"


namespace YAML
{
	class Node;
}

namespace OpenXcom
{

class Game;

namespace Lua
{

//template specialization forward declarations
template <> void toLua(lua_State* L, const YAML::Node& arg);
template <> YAML::Node fromLua(lua_State* luaState, int index);

class GameScript : public LuaApi
{
private:
	Game& _game;

	GeoscapeScript _geoscapeScript;
	BattlescapeScript _battlescapeScript;
	BasescapeScript _basescapeScript;

	using LuaOnLoadGame = LuaSimpleCallback<void, const YAML::Node&>;
	using LuaOnSaveGame = LuaAccumulatorCallback<YAML::Node, const YAML::Node&>;

	LuaOnLoadGame _onLoadGame;
	LuaOnSaveGame _onSaveGame;

public:
	GameScript(Game& game);
	~GameScript();

	/// Registers the game API with the Lua state.
	virtual void onRegisterApi(lua_State* luaState, int parentTableIndex) override;

	inline GeoscapeScript& getGeoscapeScript() { return _geoscapeScript; }
	inline BattlescapeScript& getBattlescapeScript() { return _battlescapeScript; }
	inline BasescapeScript& getBasescapeScript() { return _basescapeScript; }

	LuaOnLoadGame& onLoadGame() { return _onLoadGame; }
	LuaOnSaveGame& onSaveGame() { return _onSaveGame; }
};

} // namespace Lua

} // namespace OpenXcom
