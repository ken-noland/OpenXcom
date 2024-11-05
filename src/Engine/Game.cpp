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
#include "Game.h"
#include "Engine.h"
#include "State.h"

#include "Platform/WindowSystem.h"
#include "Platform/Window.h"

#include "../Menu/StartState.h"

namespace OpenXcom
{

// Function that returns a reference to a thread-local Game* pointer
Game*& _GamePtr()
{
	static thread_local Game* ptr = nullptr;
	return ptr;
}

void setThreadLocalGame(Game* gameInstance)
{
	_GamePtr() = gameInstance;
}

Game* getGame()
{
	return _GamePtr();
}

/**
 * Constructor for the game class.
 * @param title Title of the game window.
 * @param options Reference to the game options.
 */
Game::Game(const std::string& title)
{
	Engine& engine = getEngine();
	_window = engine.getPlatformWindowSystem().createWindow(title, 1024, 768);
	std::shared_ptr<PlatformWindow> window = _window.lock();

	// TODO: when the game window closes, send the application termination message

	// set the initial game state
	setState(std::make_unique<StartState>());
}

/**
 * Deletes the display screen, cursor, states and shuts down all the SDL subsystems.
 */
Game::~Game()
{
}

/**
 * The state machine takes care of passing all the events from SDL to the
 * active state, running any code within and blitting all the states and
 * cursor to the screen. This is run indefinitely until the game quits.
 */
void Game::update()
{
	//while (_window.isRunning())
	//{
	//	_window.processEvents();
	//	//for (State* state : _states)
	//	//{
	//	//	state->update();
	//	//}
	//	//// clear the deleted states
	//	//_deleted.clear();

	//	#if defined(ENABLE_ENTITY_INSPECTOR)
	//	_inspector.update();
	//	#endif
	//}
}

/**
 * Stops the state machine and the game is shut down.
 */
void Game::quit()
{
}

/**
 * Pops all the states currently in stack and pushes in the new state.
 * A shortcut for cleaning up all the old states when they're not necessary
 * like in one-way transitions.
 * @param state Pointer to the new state.
 */
void Game::setState(std::unique_ptr<State> state)
{
	while (!_states.empty())
	{
		popState();
	}
	pushState(std::move(state));
}

/**
 * Pushes a new state into the top of the stack and initializes it.
 * The new state will be used once the next game cycle starts.
 * @param state Pointer to the new state.
 */
void Game::pushState(std::unique_ptr<State> state)
{
	_states.push_back(std::move(state));
}

/**
 * Pops the last state from the top of the stack. Since states
 * can't actually be deleted mid-cycle, it's moved into a separate queue
 * which is cleared at the start of every cycle, so the transition
 * is seamless.
 */
void Game::popState()
{
	_deleted.push_back(std::move(_states.back()));
	_states.pop_back();
}

State* Game::getState()
{
	return _states.back().get();
}

}
