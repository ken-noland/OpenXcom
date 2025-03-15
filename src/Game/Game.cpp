#include "Game.h"
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
#include "../Engine/Graphics/Common/GameSurface.h"
#include "../Engine/Graphics/Common/WindowSurface.h"

#include "../Engine/Engine.h"
#include "../Engine/Options.h"
#include "../Engine/Platform/PlatformProcessSystem.h"

#include "States/State.h"
#include "States/StartState.h"

#include "GameWindow.h"
#include "GameStates.h"
#include "GameMods.h"

#if defined(ENABLE_ENTITY_INSPECTOR)
#include "Inspector/Inspector.h"
#endif

namespace OpenXcom
{

Game::Game(Engine& engine)
	: _engine(engine)
{
	EngineContext& engineContext = _engine.getEngineContext();

	_gameContext = std::make_unique<GameContext>(engineContext);

	_gameWindow = std::make_unique<GameWindow>(engineContext);
	_gameContext->setGameWindow(*_gameWindow);

	_gameStates = std::make_unique<GameStates>();
	_gameContext->setGameStates(*_gameStates);

	_gameMods = std::make_unique<GameMods>(*_gameContext);
	_gameContext->setGameMods(*_gameMods);

	_onGameRenderHandle = _gameWindow->getGameSurface().onRender().add([this](GraphicsCommand& command) {
		this->onGameRender(command);
	});

	_onWindowRenderHandle = _gameWindow->getGameSurface().onRender().add([this](GraphicsCommand& command) {
		this->onWindowRender(command);
	});

	// Set the initial game state to the start state
	_gameStates->set(std::make_unique<StartState>(*_gameContext));

#if defined(ENABLE_ENTITY_INSPECTOR)
	_inspector = std::make_unique<Inspector>(*_gameContext);
	_inspector->show();
#endif()
}

/**
 * Deletes the game
 */
Game::~Game()
{	
	_gameStates->clear();

#if defined(ENABLE_ENTITY_INSPECTOR)
	_inspector.reset();
#endif()

	_onGameRenderHandle.reset();
	_onWindowRenderHandle.reset();

	_gameWindow.reset();
}

int Game::run()
{
	// some options (like -version or -help) don't need to run the game
	if (_engine.getOptions().get<&GameOptions::_shouldRun>() == false)
	{
		return EXIT_SUCCESS;
	}

	
	// run until the game is done
	while (_engine.getPlatformProcessSystem().isRunning() == true && isRunning())
	{
		_engine.update();

		update();
	}

	return EXIT_SUCCESS;

}

void Game::onGameRender(GraphicsCommand& command)
{
	_gameStates->render(command);
}

void Game::onWindowRender(GraphicsCommand& command)
{
}

void Game::update()
{
	_gameWindow->update();
	_gameStates->update();

#if defined(ENABLE_ENTITY_INSPECTOR)
	_inspector->update();
#endif()

}

/**
 * Stops the state machine and the game is shut down.
 */
void Game::quit()
{
}

bool Game::load()
{
	return _gameMods->load();
}

bool Game::isRunning() const
{
	return _gameWindow->isRunning();
}

GameContext& Game::getGameContext()
{
	return *_gameContext;
}

}
