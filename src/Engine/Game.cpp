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
#include "Options.h"
#include "State.h"

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
 * Starts up all the SDL subsystems,
 * creates the display screen and sets up the cursor.
 * also creates the base game lua state.
 * @param title Title of the game window.
 */
Game::Game(const std::string& title, Options& options)
	: _options(options), _vfs(options)
{
	setThreadLocalGame(this);

	// gmae owns vitual filesystem


	//Options::reload = false;
	//Options::mute = false;

	//// Initialize SDL
	//if (SDL_Init(SDL_INIT_VIDEO) < 0)
	//{
	//	Log(LOG_ERROR) << SDL_GetError();
	//	Log(LOG_WARNING) << "No video detected, quit.";
	//	throw Exception(SDL_GetError());
	//}
	//Log(LOG_INFO) << "SDL initialized successfully.";

	//// Initialize SDL_mixer
	//initAudio();

	//// trap the mouse inside the window
	//SDL_WM_GrabInput(Options::captureMouse);

	//// Set the window icon
	//CrossPlatform::setWindowIcon(IDI_ICON1, "openxcom.png");

	//// Set the window caption
	//SDL_WM_SetCaption(title.c_str(), 0);

	//// Set up unicode
	//SDL_EnableUNICODE(1);
	//Unicode::getUtf8Locale();

	//// Create display
	//_screen = new Screen();

	//// Create cursor
	//_cursor = new Cursor(9, 13);

	//// Create invisible hardware cursor to workaround bug with absolute positioning pointing devices
	//SDL_ShowCursor(SDL_ENABLE);
	//uint8_t cursor = 0;
	//SDL_SetCursor(SDL_CreateCursor(&cursor, &cursor, 1,1,0,0));

	//// Create fps counter
	//_fpsCounter = new FpsCounter(15, 5, 0, 0);

	//// Create blank language
	//_lang = new Language();

	//_timeOfLastFrame = 0;

	
	#if defined(ENABLE_ENTITY_INSPECTOR)
	_inspector.create();
	#endif
}

/**
 * Deletes the display screen, cursor, states and shuts down all the SDL subsystems.
 */
Game::~Game()
{
	#if defined(ENABLE_ENTITY_INSPECTOR)
	_inspector.destroy();
	#endif

	//Sound::stop();
	//Music::stop();

	//for (auto* state : _states)
	//{
	//	delete state;
	//}

	//SDL_FreeCursor(SDL_GetCursor());

	//delete _cursor;
	//delete _lang;
	//delete _save;
	//_mod.reset();
	//delete _screen;
	//delete _fpsCounter;

	//Mix_CloseAudio();

	//SDL_Quit();
}

/**
 * The state machine takes care of passing all the events from SDL to the
 * active state, running any code within and blitting all the states and
 * cursor to the screen. This is run indefinitely until the game quits.
 */
void Game::run()
{
}

/**
 * Stops the state machine and the game is shut down.
 */
void Game::quit()
{
	// Hard-learned lesson: there's a billion+ situations, where this causes a corrupted save and subsequent crashes. It's not worth it!
#if 0
	// Always save ironman
	if (_save != 0 && _save->isIronman() && !_save->getName().empty())
	{
		std::string filename = CrossPlatform::sanitizeFilename(_save->getName()) + ".sav";
		_save->save(filename, _mod);
	}
#endif
//	_quit = true;
}

/**
 * Pops all the states currently in stack and pushes in the new state.
 * A shortcut for cleaning up all the old states when they're not necessary
 * like in one-way transitions.
 * @param state Pointer to the new state.
 */
void Game::setState(State *state)
{
	while (!_states.empty())
	{
		popState();
	}
	pushState(state);
}

/**
 * Pushes a new state into the top of the stack and initializes it.
 * The new state will be used once the next game cycle starts.
 * @param state Pointer to the new state.
 */
void Game::pushState(State *state)
{
	_states.push_back(state);
}

/**
 * Pops the last state from the top of the stack. Since states
 * can't actually be deleted mid-cycle, it's moved into a separate queue
 * which is cleared at the start of every cycle, so the transition
 * is seamless.
 */
void Game::popState()
{
	_deleted.push_back(_states.back());
	_states.pop_back();
}

State* Game::getState()
{
	return _states.back();
}

const std::list<State*>& Game::getStates() const
{
	return _states;
}

}
