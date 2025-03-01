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
#include <gtest/gtest.h>

#include "../../Game/Game.h"
#include "../../Game/GameMods.h"
#include "../../Game/GameContext.h"
#include "../../Engine/EngineContext.h"
#include "../../Engine/Engine.h"

#include "../../Engine/Options.h"
#include "../../Engine/Filesystem/VirtualFileSystem.h"
#include "../../Engine/Platform/ProcessSystem.h"
#include "../../Engine/Graphics/Null/NullGraphicsSystem.h"
#include "../../Engine/Resource/ResourceSystem.h"
#include "../../Engine/Utility/Time/TimeSystem.h"

using namespace OpenXcom;

class TestEngineContext : public EngineContext
{
public:
};

class TestEngine : public Engine
{
public:
	TestEngine()
	{
		_engineContext = std::make_unique<TestEngineContext>(*this);

		// empty options
		_options = std::make_unique<Options>();
		_engineContext->setOptions(_options.get());

		selectOptions();

		// empty virtual file system
		_virtualFileSystem = std::make_unique<VirtualFileSystem>(*_options);
		_engineContext->setVirtualFileSystem(_virtualFileSystem.get());

		// empty process system
		_platformProcessSystem = std::make_unique<PlatformProcessSystem>();
		_engineContext->setPlatformProcessSystem(_platformProcessSystem.get());

		// null graphics system
		_graphicsSystem = std::make_unique<NullGraphicsSystem>(*_engineContext);
		_engineContext->setGraphicsSystem(_graphicsSystem.get());

		// empty resource system
		_resourceSystem = std::make_unique<ResourceSystem>(*_engineContext);
		_engineContext->setResourceSystem(_resourceSystem.get());
	}

	void selectOptions()
	{
		std::filesystem::path path = TEST_DATA_DIR;
		std::vector<std::filesystem::path> dataPaths = {path / "data"};
		std::filesystem::path configPath = path / "config";
		std::filesystem::path userPath = path / "user";

		// set some test options
		_options->set<&GameOptions::_dataPath>(OptionLevel::COMMAND, dataPaths);
		_options->set<&GameOptions::_userPath>(OptionLevel::COMMAND, userPath);
		_options->set<&GameOptions::_cfgPath>(OptionLevel::COMMAND, configPath);
	}

	~TestEngine() = default;
};

class TestGameContext : public GameContext
{
public:
	TestGameContext(EngineContext& engine, Game* game) : GameContext(engine, game) {}
	~TestGameContext() = default;

	void setGameWindow(GameWindow& gameWindow) { _gameWindow = &gameWindow; }
	void setGameStates(GameStates& gameStates) { _gameStates = &gameStates; }
	void setGameMods(GameMods& gameMods) { _gameMods = &gameMods; }
};

class TestGame : public Game
{
public:
	TestGame(Engine& engine) : Game(engine)
	{
	}

	~TestGame() = default;
};


TEST(TestGameMods, TestGameMods)
{
	TestEngine engine;
	TestGame game(engine);

//	GameMods gameMods(gameContext);
}
