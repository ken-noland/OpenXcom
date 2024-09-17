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
#include <list>
#include <string>
#include <memory>

#include "../Entity/Engine/ECS.h"

#if defined(ENABLE_ENTITY_INSPECTOR)
#include "../Inspector/Inspector.h"
#endif

namespace OpenXcom
{

class State;
class Options;

namespace Lua
{
class LuaMod;
}

/**
 * The core of the game engine, manages the game's entire contents and structure.
 * Takes care of encapsulating all the core SDL commands, provides access to all
 * the game's resources and contains a stack state machine to handle all the
 * initializations, events and blits of each state, as well as transitions.
 */
class Game
{
private:

	std::list<State*> _states, _deleted;

	/// central entity component system
	ECS _ecs;

	#if defined(ENABLE_ENTITY_INSPECTOR)
	Inspector _inspector;
	#endif

public:
	/// Creates a new game.
	Game(const std::string &title, Options* options);
	/// Cleans up all the game's resources.
	~Game();

	/// Starts the game's state machine.
	void run();
	/// Quits the game.
	void quit();

	/// Resets the state stack to a new state.
	void setState(State *state);
	/// Pushes a new state into the state stack.
	void pushState(State *state);
	/// Pops the last state from the state stack.
	void popState();
	/// Gets the last state from the state stack
	State* getState();
	/// Gets the state stack
	const std::list<State*>& getStates() const;

	/// Gets the registry container
	const [[nodiscard]] ECS& getECS() const { return _ecs; }
	/// Gets the registry container
	[[nodiscard]] ECS& getECS() { return _ecs; }
};

/// Global function that retrieve a thread local Game object.
Game* getGame();


}
