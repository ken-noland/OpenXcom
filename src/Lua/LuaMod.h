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
#include "LuaState.h"
#include <memory>

namespace OpenXcom
{

class Game;
class ModFile;

namespace Lua
{

class GameScript;

/// 
class LuaMod
{
private:
	Game& _game;
	const ModFile& _modFiles;

	using LuaMods = std::vector<std::unique_ptr<LuaState>>;
	LuaMods _luaMods;

	std::unique_ptr<GameScript> _luaGameScript;

public:
	LuaMod(Game& game, const ModFile& modFiles);
	~LuaMod();

	bool loadAll();

	inline GameScript& getGameScript() const { return *_luaGameScript; }

};

} // namespace Lua
} // namespace OpenXcom
