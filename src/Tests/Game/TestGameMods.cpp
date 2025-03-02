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
#include "../../Engine/Platform/PlatformProcessSystem.h"
#include "../../Engine/Platform/PlatformWindowSystem.h"
#include "../../Engine/Graphics/Null/NullGraphicsSystem.h"
#include "../../Engine/Resource/ResourceSystem.h"
#include "../../Engine/Utility/Time/TimeSystem.h"

using namespace OpenXcom;

class TestEngine : public Engine
{
public:
	TestEngine()
	{
		_engineContext = std::make_unique<EngineContext>(*this);

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

		// null window system
		_platformWindowSystem = std::make_unique<PlatformWindowSystem>();
		_engineContext->setPlatformWindowSystem(_platformWindowSystem.get());

		// null graphics system
		_graphicsSystem = std::make_unique<NullGraphicsSystem>(*_engineContext);
		_engineContext->setGraphicsSystem(_graphicsSystem.get());

		// empty resource system
		_resourceSystem = std::make_unique<ResourceSystem>(*_engineContext);
		_engineContext->setResourceSystem(_resourceSystem.get());
	}

	void selectOptions()
	{
		std::filesystem::path core_path = CORE_DATA_DIR;
		std::filesystem::path test_path = TEST_DATA_DIR;

		std::vector<std::filesystem::path> dataPaths = { core_path / "data", test_path / "data" };
		std::filesystem::path configPath = test_path / "config";
		std::filesystem::path userPath = test_path / "user";

		// set some test options
		_options->set<&GameOptions::_dataPath>(OptionLevel::COMMAND, dataPaths);
		_options->set<&GameOptions::_userPath>(OptionLevel::COMMAND, userPath);
		_options->set<&GameOptions::_cfgPath>(OptionLevel::COMMAND, configPath);
	}

	~TestEngine() = default;
};

// trimmed up version of the base game
class TestGame
{
protected:
	GameContext _gameContext;

	std::unique_ptr<GameMods> _gameMods;

public:
	TestGame(Engine& engine)
		: _gameContext(engine.getEngineContext())
	{
		_gameMods = std::make_unique<GameMods>(_gameContext);
		_gameContext.setGameMods(*_gameMods);
	}
	~TestGame() = default;

	bool load()
	{
		return _gameMods->load();
	}
};


TEST(TestGameMods, TestGameMods)
{
	TestEngine engine;
	TestGame game(engine);

	ASSERT_TRUE(game.load());
}
