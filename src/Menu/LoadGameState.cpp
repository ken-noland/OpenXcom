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
#include "LoadGameState.h"
#include <sstream>
#include "../Engine/Logger.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Engine/Game.h"
#include "../Engine/Exception.h"
#include "../Engine/Options.h"
#include "../Engine/CrossPlatform.h"
#include "../Engine/Screen.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/Text.h"
#include "../Geoscape/GeoscapeState.h"
#include "ErrorMessageState.h"
#include "../Battlescape/BattlescapeState.h"
#include "../Mod/Mod.h"
#include "../Engine/Sound.h"
#include "../Engine/Unicode.h"
#include "../Mod/RuleInterface.h"
#include "../Lua/LuaMod.h"
#include "../Lua/GameScript.h"
#include "StatisticsState.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Load Game screen.
 * @param game Pointer to the core game.
 * @param origin Game section that originated this state.
 * @param filename Name of the save file without extension.
 * @param palette Parent state palette.
 */
LoadGameState::LoadGameState(OptionsOrigin origin, const std::string &filename, SDL_Color *palette)
	: State("LoadGameState", false), _firstRun(0), _origin(origin), _filename(filename)
{
	buildUi(palette);
}

/**
 * Initializes all the elements in the Load Game screen.
 * @param game Pointer to the core game.
 * @param origin Game section that originated this state.
 * @param type Type of auto-load being used.
 * @param palette Parent state palette.
 */
LoadGameState::LoadGameState(OptionsOrigin origin, SaveType type, SDL_Color* palette)
	: State("LoadGameState", false), _firstRun(0), _origin(origin)
{
	switch (type)
	{
	case SAVE_QUICK:
		_filename = SavedGame::QUICKSAVE;
		break;
	case SAVE_AUTO_GEOSCAPE:
		_filename = SavedGame::AUTOSAVE_GEOSCAPE;
		break;
	case SAVE_AUTO_BATTLESCAPE:
		_filename = SavedGame::AUTOSAVE_BATTLESCAPE;
		break;
	default:
		// can't auto-load ironman games
		break;
	}

	buildUi(palette);
}

/**
 *
 */
LoadGameState::~LoadGameState()
{

}

/**
 * Builds the interface.
 * @param palette Parent state palette.
 */
void LoadGameState::buildUi(SDL_Color *palette)
{
	// Create objects
	_txtStatus = new Text(320, 17, 0, 92);

	// Set palette
	setStatePalette(palette);

	if (_origin == OPT_BATTLESCAPE)
	{
		add(_txtStatus, "textLoad", "battlescape");
		_txtStatus->setHighContrast(true);
		if (getGame()->getSavedGame()->getSavedBattle()->getAmbientSound() != Mod::NO_SOUND)
		{
			getGame()->getMod()->getSoundByDepth(0, getGame()->getSavedGame()->getSavedBattle()->getAmbientSound())->stopLoop();
		}
	}
	else
	{
		add(_txtStatus, "textLoad", "geoscape");
	}

	centerAllSurfaces();

	// Set up objects
	_txtStatus->setBig();
	_txtStatus->setAlign(ALIGN_CENTER);
	_txtStatus->setText(tr("STR_LOADING_GAME"));

}

/**
 * Ignore quick loads without a save available.
 */
void LoadGameState::init()
{
	State::init();
	if (_filename == SavedGame::QUICKSAVE && !CrossPlatform::fileExists(Options::getMasterUserFolder() + _filename))
	{
		getGame()->popState();
		return;
	}
}

/**
 * Loads the specified save.
 */
void LoadGameState::think()
{
	State::think();
	// Make sure it gets drawn properly
	if (_firstRun < 10)
	{
		_firstRun++;
	}
	else
	{
		getGame()->popState();

		// Remember for later (palette reset)
		BattlescapeState *origBattleState = 0;
		if (getGame()->getSavedGame() != 0 && getGame()->getSavedGame()->getSavedBattle() != 0)
		{
			origBattleState = getGame()->getSavedGame()->getSavedBattle()->getBattleState();
		}

		// Reset touch flags
		getGame()->resetTouchButtonFlags();

		// Load the game
		SavedGame *s = new SavedGame();
		try
		{
			YAML::Node save;
			s->load(_filename, getGame()->getMod(), getGame()->getLanguage(), save);
			getGame()->setSavedGame(s);

			
			getGame()->getLuaMod().getGameScript().onLoadGame().dispatchCallback(save);


			if (getGame()->getSavedGame()->getEnding() != END_NONE)
			{
				Options::baseXResolution = Screen::ORIGINAL_WIDTH;
				Options::baseYResolution = Screen::ORIGINAL_HEIGHT;
				getGame()->getScreen()->resetDisplay(false);
				getGame()->setState(new StatisticsState);
			}
			else
			{
				Options::baseXResolution = Options::baseXGeoscape;
				Options::baseYResolution = Options::baseYGeoscape;
				getGame()->getScreen()->resetDisplay(false);
				if (origBattleState != 0)
				{
					// We need to reset palettes here already, can't wait for the destructor
					origBattleState->resetPalettes();
				}
				getGame()->setState(new GeoscapeState);
				if (getGame()->getSavedGame()->getSavedBattle() != 0)
				{
					getGame()->getSavedGame()->getSavedBattle()->loadMapResources(getGame()->getMod());
					Options::baseXResolution = Options::baseXBattlescape;
					Options::baseYResolution = Options::baseYBattlescape;
					getGame()->getScreen()->resetDisplay(false);
					BattlescapeState *bs = new BattlescapeState;
					getGame()->pushState(bs);
					getGame()->getSavedGame()->getSavedBattle()->setBattleState(bs);
				}
			}

			// Clear the SDL event queue (i.e. ignore input from impatient users)
			SDL_Event e;
			while (SDL_PollEvent(&e))
			{
				// do nothing
			}
		}
		catch (Exception &e)
		{
			error(e.what(), s);
		}
		catch (YAML::Exception &e)
		{
			error(e.what(), s);
		}
		CrossPlatform::flashWindow();
	}
}

/**
 * Pops up a window with an error message
 * and cleans up afterwards.
 * @param msg Error message.
 * @param save Pending save.
 */
void LoadGameState::error(const std::string &msg, SavedGame *save)
{

	Log(LOG_ERROR) << msg;
	std::ostringstream error;
	error << tr("STR_LOAD_UNSUCCESSFUL") << Unicode::TOK_NL_SMALL << msg;
	if (_origin != OPT_BATTLESCAPE)
		getGame()->pushState(new ErrorMessageState(error.str(), _palette, getGame()->getMod()->getInterface("errorMessages")->getElement("geoscapeColor")->color, "BACK01.SCR", getGame()->getMod()->getInterface("errorMessages")->getElement("geoscapePalette")->color));
	else
		getGame()->pushState(new ErrorMessageState(error.str(), _palette, getGame()->getMod()->getInterface("errorMessages")->getElement("battlescapeColor")->color, "TAC00.SCR", getGame()->getMod()->getInterface("errorMessages")->getElement("battlescapePalette")->color));

	if (getGame()->getSavedGame() == save)
		getGame()->setSavedGame(0);
	else
		delete save;
}

}
