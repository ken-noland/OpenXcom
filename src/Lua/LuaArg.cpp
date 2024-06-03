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

void yamlNodeToLua(lua_State* L, const YAML::Node& node)
{
	if (node.IsScalar())
	{
		toLuaArg<const std::string&>(L, node.as<std::string>());
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

} // namespace Lua

} // namespace OpenXcom
