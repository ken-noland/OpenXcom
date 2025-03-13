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
template <typename _GameMods>
class TestGame
{
protected:
	GameContext _gameContext;

	std::unique_ptr<_GameMods> _gameMods;

public:
	TestGame(Engine& engine)
		: _gameContext(engine.getEngineContext())
	{
		_gameMods = std::make_unique<_GameMods>(_gameContext);
		_gameContext.setGameMods(*_gameMods);
	}
	~TestGame() = default;

	bool load()
	{
		return _gameMods->load();
	}

	_GameMods& getGameMods() { return *_gameMods; }
};

class TestGameMods : public GameMods
{
public:
	TestGameMods(GameContext& gameContext)
		: GameMods(gameContext)
	{
	}

	~TestGameMods() = default;

	void addScannedMod(const ModInfo& modInfo)
	{
		// we need to add a filesystem here, because mods specified this way do not have a folder which we scanned ModInfo from
		// so we need to add a dummy filesystem to the mod
		std::unique_ptr<CompositeFileSystem> filesystem = std::make_unique<CompositeFileSystem>();
		filesystem->addFileSystem(std::make_unique<PhysicalFileSystem>(TEST_DATA_DIR));

		_scannedMods.inactiveMods.push_back(ScannedMod{modInfo, TEST_DATA_DIR, std::move(filesystem)});
	}

};

TEST(TestGameMods, TestGameXcom1)
{
	TestEngine engine;
	TestGame<TestGameMods> game(engine);

	// set the master and mods to load in the config
	Options& options = engine.getOptions();
	options.set<&GameOptions::_master>(OptionLevel::COMMAND, "xcom1");

	ASSERT_TRUE(game.load());

	// now lets go through the resources loaded and see if we can extract some resources to verify they are present

}

TEST(TestGameMods, TestGameXcom2)
{
	TestEngine engine;
	TestGame<TestGameMods> game(engine);

	// set the master and mods to load in the config
	Options& options = engine.getOptions();
	options.set<&GameOptions::_master>(OptionLevel::COMMAND, "xcom2");

	ASSERT_TRUE(game.load());
}

TEST(TestGameMods, TestGameModDependencies)
{
	TestEngine engine;
	TestGame<TestGameMods> game(engine);

	// Add three mods. The first is a master, the second depends on the first, and the third depends on the first.
	game.getGameMods().addScannedMod(ModInfo{"master1_id", "master1 name", "master1 description", "master1 author",
		semver::version(0, 1, 0), ModType::Master, "required_engine", semver::version(0, 1, 0), semver::version(8, 1, 2),
		{/* dependencies */}, {/* conflicts */}, "", {"UFO"}});
	game.getGameMods().addScannedMod(ModInfo{"mod1_id", "mod1 name", "mod1 description", "mod1 author",
		semver::version(0, 1, 0), ModType::Mod, "required_engine", semver::version(0, 1, 0), semver::version(8, 1, 2),
		{DependencyExpression{"master1_id"}}, {/* conflicts */}, "", {""}});
	game.getGameMods().addScannedMod(ModInfo{"mod2_id", "mod2 name", "mod2 description", "mod2 author",
		semver::version(0, 1, 0), ModType::Mod, "required_engine", semver::version(0, 1, 0), semver::version(8, 1, 2),
		{DependencyExpression{"master1_id"}}, {/* conflicts */}, "", {""}});

	// set the master and mods to load in the config
	Options& options = engine.getOptions();
	options.set<&GameOptions::_master>(OptionLevel::COMMAND, "master1_id");
	options.set<&GameOptions::_mods>(OptionLevel::COMMAND, {"mod1_id", "mod2_id"});
	options.set<&GameOptions::_scanFilesystem>(OptionLevel::COMMAND, false);

	// see if they load
	ASSERT_TRUE(game.load());
}

TEST(TestGameMods, TestMissingDependencies)
{
	TestEngine engine;
	TestGame<TestGameMods> game(engine);

	// Add three mods. The first is a master, the second depends on the first, and the third depends on the first.
	game.getGameMods().addScannedMod(ModInfo{"master1_id", "master1 name", "master1 description", "master1 author",
		semver::version(0, 1, 0), ModType::Master, "required_engine", semver::version(0, 1, 0), semver::version(8, 1, 2),
		{/* dependencies */}, {/* conflicts */}, "", {""}});
	game.getGameMods().addScannedMod(ModInfo{"mod1_id", "mod1 name", "mod1 description", "mod1 author",
		semver::version(0, 1, 0), ModType::Mod, "required_engine", semver::version(0, 1, 0), semver::version(8, 1, 2),
		{DependencyExpression{"master1_id"}}, {/* conflicts */}, "", {""}});
	game.getGameMods().addScannedMod(ModInfo{"mod2_id", "mod2 name", "mod2 description", "mod2 author",
		semver::version(0, 1, 0), ModType::Mod, "required_engine", semver::version(0, 1, 0), semver::version(8, 1, 2),
		{DependencyExpression{"master1_id"}, DependencyExpression{"missing_mod_id"}}, {/* conflicts */}, "", {""}});

	// set the master and mods to load in the config
	Options& options = engine.getOptions();
	options.set<&GameOptions::_master>(OptionLevel::COMMAND, "master1_id");
	options.set<&GameOptions::_mods>(OptionLevel::COMMAND, {"mod1_id", "mod2_id"});
	options.set<&GameOptions::_scanFilesystem>(OptionLevel::COMMAND, false);

	// see if they load
	ASSERT_FALSE(game.load());
}

TEST(TestGameMods, TestConflictingDependencies1)
{
	TestEngine engine;
	TestGame<TestGameMods> game(engine);

	// Add three mods. The first is a master, the second depends on the first, and the third depends on the first.
	game.getGameMods().addScannedMod(ModInfo{"master1_id", "master1 name", "master1 description", "master1 author",
		semver::version(0, 1, 0), ModType::Master, "required_engine", semver::version(0, 1, 0), semver::version(8, 1, 2),
		{/* dependencies */}, {/* conflicts */}, "", {""}});
	game.getGameMods().addScannedMod(ModInfo{"mod1_id", "mod1 name", "mod1 description", "mod1 author",
		semver::version(0, 1, 0), ModType::Mod, "required_engine", semver::version(0, 1, 0), semver::version(8, 1, 2),
		{DependencyExpression{"master1_id"}}, {/* conflicts */}, "", {""}});
	game.getGameMods().addScannedMod(ModInfo{"mod2_id", "mod2 name", "mod2 description", "mod2 author",
		semver::version(0, 1, 0), ModType::Mod, "required_engine", semver::version(0, 1, 0), semver::version(8, 1, 2),
		{DependencyExpression{"master1_id"}}, {DependencyExpression{"mod1_id"}}, "", {""}});

	// set the master and mods to load in the config
	Options& options = engine.getOptions();
	options.set<&GameOptions::_master>(OptionLevel::COMMAND, "master1_id");
	options.set<&GameOptions::_mods>(OptionLevel::COMMAND, {"mod1_id", "mod2_id"});
	options.set<&GameOptions::_scanFilesystem>(OptionLevel::COMMAND, false);

	// see if they load
	ASSERT_FALSE(game.load());
}

TEST(TestGameMods, TestConflictingDependencies2)
{
	TestEngine engine;
	TestGame<TestGameMods> game(engine);

	// Add three mods. The first is a master, the second depends on the first, and the third depends on the first.
	game.getGameMods().addScannedMod(ModInfo{"master1_id", "master1 name", "master1 description", "master1 author",
		semver::version(0, 1, 0), ModType::Master, "required_engine", semver::version(0, 1, 0), semver::version(8, 1, 2),
		{/* dependencies */}, {/* conflicts */}, "", {""}});
	game.getGameMods().addScannedMod(ModInfo{"mod1_id", "mod1 name", "mod1 description", "mod1 author",
		semver::version(0, 1, 0), ModType::Mod, "required_engine", semver::version(0, 1, 0), semver::version(8, 1, 2),
		{DependencyExpression{"master1_id"}}, {DependencyExpression{"mod2_id"}}, "", {""}});
	game.getGameMods().addScannedMod(ModInfo{"mod2_id", "mod2 name", "mod2 description", "mod2 author",
		semver::version(0, 1, 0), ModType::Mod, "required_engine", semver::version(0, 1, 0), semver::version(8, 1, 2),
		{DependencyExpression{"master1_id"}}, {/* conflicts */}, "", {""}});

	// set the master and mods to load in the config
	Options& options = engine.getOptions();
	options.set<&GameOptions::_master>(OptionLevel::COMMAND, "master1_id");
	options.set<&GameOptions::_mods>(OptionLevel::COMMAND, {"mod1_id", "mod2_id"});
	options.set<&GameOptions::_scanFilesystem>(OptionLevel::COMMAND, false);

	// see if they load
	ASSERT_FALSE(game.load());
}
