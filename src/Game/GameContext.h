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

namespace OpenXcom
{

class EngineContext;
class GameWindow;
class GameStates;
class GameMods;
class Game;

class GameContext
{
protected:
	EngineContext& _engine;
	Game* _game;

	GameWindow* _gameWindow;
	GameStates* _gameStates;
	GameMods* _gameMods;

	friend class Game;
	void setGameWindow(GameWindow& gameWindow) { _gameWindow = &gameWindow; }
	void setGameStates(GameStates& gameStates) { _gameStates = &gameStates; }
	void setGameMods(GameMods& gameMods) { _gameMods = &gameMods; }

public:
	GameContext(EngineContext& engine, Game* game)
		: _engine(engine), _game(game) { }
	~GameContext() {}

	EngineContext& getEngineContext() { return _engine; }
	Game& getGame() { return *_game; }

	GameWindow& getGameWindow() { return *_gameWindow; }
	GameStates& getGameStates() { return *_gameStates; }
	GameMods& getGameMods() { return *_gameMods; }
};

} // namespace OpenXcom
