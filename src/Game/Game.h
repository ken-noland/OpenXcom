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

#include "GameContext.h"

#include "../Engine/Filesystem/VirtualFileSystem.h"
#include "../Engine/Platform/Window.h"
#include "../Entity/Engine/ECS.h"

#if defined(ENABLE_ENTITY_INSPECTOR)
#include "../Inspector/Inspector.h"
#endif


////////////////////////////////////////
// Begin Temp
//
// putting this here for now, but the file processor should be moved to a real loader
////////////////////////////////////////
#include "Resource/Xcom1MasterFileProcessor.h"
////////////////////////////////////////
// End Temp
////////////////////////////////////////


namespace OpenXcom
{

class Engine;
class State;
class Options;
class GameWindow;
class GameStates;
class GraphicsCommand;

/**
 * The core of the game engine, manages the game's entire contents and structure.
 * Takes care of encapsulating all the core game systems, provides access to all
 * the game's resources and contains a stack state machine to handle all the
 * initializations.
 */
class Game
{
protected:
	// Game engine
	Engine& _engine;

	// The game's context.
	std::unique_ptr<GameContext> _gameContext;

	// The game's states.
	std::unique_ptr<GameStates> _gameStates;

	// The game's window.
	std::unique_ptr<GameWindow> _gameWindow;

	// Handles to delegates for rendering
	MulticastDelegate<void(GraphicsCommand&)>::Handle _onWindowRenderHandle;
	MulticastDelegate<void(GraphicsCommand&)>::Handle _onGameRenderHandle;

#if defined(ENABLE_ENTITY_INSPECTOR)
	Inspector _inspector;
#endif

	void onGameRender(GraphicsCommand& command);
	void onWindowRender(GraphicsCommand& command);

	////////////////////////////////////////
	// Begin Temp
	////////////////////////////////////////
	Xcom1MasterFileProcessor _masterFileProcessor;
	////////////////////////////////////////
	// End Temp
	////////////////////////////////////////


public:
	/// Creates a new game.
	Game(Engine& engine);
	/// Cleans up all the game's resources.
	~Game();

	int run();
	void update();
	void quit();

	bool load();

	bool isRunning() const;
	GameContext& getGameContext();
};

}
