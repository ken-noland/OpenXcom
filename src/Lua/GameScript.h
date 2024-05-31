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

#include "GeoscapeScript.h"
#include "BattlescapeScript.h"
#include "BasescapeScript.h"


extern "C"
{
	struct lua_State;
}

namespace OpenXcom
{

namespace Lua
{

class GameScript
{
private:
	GeoscapeScript _geoscapeScript;
	BattlescapeScript _battlescapeScript;
	BasescapeScript _basescapeScript;

public:
	GameScript();
	~GameScript();

	/// Populates the Lua state with the game API.
	bool Populate(lua_State* luaState);

	inline GeoscapeScript& getGeoscapeScript() { return _geoscapeScript; }
	inline BattlescapeScript& getBattlescapeScript() { return _battlescapeScript; }
	inline BasescapeScript& getBasescapeScript() { return _basescapeScript; }
};

} // namespace Lua

} // namespace OpenXcom
