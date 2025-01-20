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

#include "../Engine/Filesystem/VirtualFileSystem.h"
#include "../Engine/Platform/Window.h"
#include "../Entity/Engine/ECS.h"

#if defined(ENABLE_ENTITY_INSPECTOR)
#include "../Inspector/Inspector.h"
#endif

namespace OpenXcom
{

class State;
class Options;
class GameWindow;

/**
 * The core of the game engine, manages the game's entire contents and structure.
 * Takes care of encapsulating all the core game systems, provides access to all
 * the game's resources and contains a stack state machine to handle all the
 * initializations.
 */
class Game
{
private:
	/// central entity component system
	ECS _ecs;


	/// The game's state stack.
	std::list<std::unique_ptr<State>> _states, _deleted;


	/// The game's window.
	std::unique_ptr<GameWindow> _gameWindow;

	#if defined(ENABLE_ENTITY_INSPECTOR)
	Inspector _inspector;
	#endif

public:
	/// Creates a new game.
	Game(const std::string &title);
	/// Cleans up all the game's resources.
	~Game();

	/// Update the game.
	void update();
	/// Quits the game.
	void quit();

	/// Resets the state stack to a new state.
	void setState(std::unique_ptr<State> state);
	/// Pushes a new state into the state stack.
	void pushState(std::unique_ptr<State> state);
	/// Pops the last state from the state stack.
	void popState();
	/// Gets the last state from the state stack
	State* getState();

	// the following will be moved to GameContext

	/// Gets the registry container
	const ECS& getECS() const { return _ecs; }
	/// Gets the registry container
	ECS& getECS() { return _ecs; }
};

/// Global function that retrieve a thread local Game object.
Game* getGame();

}
