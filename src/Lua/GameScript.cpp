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
#include "../Savegame/SavedGame.h"

#include <yaml-cpp/yaml.h>

namespace OpenXcom
{

namespace Lua
{

	
void yamlNodeToLua(lua_State* L, const YAML::Node& node)
{
	if (node.IsScalar())
	{
		toLua<const std::string&>(L, node.as<std::string>());
	}
	else if (node.IsSequence())
	{
		lua_newtable(L);
		for (std::size_t i = 0; i < node.size(); ++i)
		{
			yamlNodeToLua(L, node[i]);
			lua_rawseti(L, -2, i + 1);
		}
	}
	else if (node.IsMap())
	{
		lua_newtable(L);
		for (const auto& pair : node)
		{
			yamlNodeToLua(L, pair.first);
			yamlNodeToLua(L, pair.second);
			lua_settable(L, -3);
		}
	}
	else
	{
		lua_pushnil(L);
	}
}

/// Converts a Lua table to a YAML node
YAML::Node luaToYamlNode(lua_State* luaState, int index)
{
	YAML::Node node;
	return node;
}



// specialization of template functions to allow for serialization of YAML
template <>
void toLua(lua_State* L, const YAML::Node& arg)
{
	yamlNodeToLua(L, arg);
}

template <>
YAML::Node fromLua(lua_State* luaState, int index)
{
	return luaToYamlNode(luaState, index);
}


GameScript::GameScript(Game& game)
	:
	LuaApi("game"),
	_game(game),
	_onLoadGame("on_load_game"),
	_onSaveGame("on_save_game")
{
}

GameScript::~GameScript()
{
}

void GameScript::onRegisterApi(lua_State* luaState, int parentTableIndex)
{
	_geoscapeScript.registerApi(luaState, parentTableIndex);

	_onLoadGame.registerApi(luaState, parentTableIndex);
	_onSaveGame.registerApi(luaState, parentTableIndex);

	//using lambdas to register functions for the game table.
	auto constexpr get_base_num = []() -> int { return 42; };
	createFunction<get_base_num>(luaState, "get_base_num");
}


} // namespace Lua

} // namespace OpenXcom
