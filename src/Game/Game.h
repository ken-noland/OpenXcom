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
#include "GameMods.h"

#include "../Engine/Filesystem/VirtualFileSystem.h"
#include "../Engine/Platform/PlatformWindow.h"
#include "../Entity/Engine/ECS.h"

namespace OpenXcom
{

class Engine;
class State;
class Options;
class GameWindow;
class GameStates;
class GameMods;
class GraphicsCommand;

#if defined(ENABLE_ENTITY_INSPECTOR)
class Inspector;
#endif


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

	// The game's mods
	std::unique_ptr<GameMods> _gameMods;

	// Handles to delegates for rendering
	MulticastDelegate<void(GraphicsCommand&)>::Handle _onWindowRenderHandle;
	MulticastDelegate<void(GraphicsCommand&)>::Handle _onGameRenderHandle;

#if defined(ENABLE_ENTITY_INSPECTOR)
	std::unique_ptr<Inspector> _inspector;
#endif

	void onGameRender(GraphicsCommand& command);
	void onWindowRender(GraphicsCommand& command);

public:
	/// Creates a new game.
	Game(Engine& engine);
	/// Cleans up all the game's resources.
	virtual ~Game();

	int run();
	void update();
	void quit();

	bool load();

	bool isRunning() const;
	GameContext& getGameContext();
};

} // namespace OpenXcom
