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
#include "NewGameState.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Interface/TextButton.h"
#include "../Interface/ToggleTextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Geoscape/GeoscapeState.h"
#include "../Geoscape/BuildNewBaseState.h"
#include "../Geoscape/BaseNameState.h"
#include "../Basescape/PlaceLiftState.h"
#include "../Engine/Options.h"
#include "../Entity/Common/GeoPosition.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Entity/Interface/Interface.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Difficulty window.
 * @param game Pointer to the core game.
 */
NewGameState::NewGameState() : State("NewGameState", true)
{
	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();

	// Create objects
	_window = factory.createWindow("windowName", this, 192, 180, 64, 10, WindowPopup::POPUP_VERTICAL);
	_btnBeginner = new TextButton(160, 18, 80, 32);
	_btnExperienced = new TextButton(160, 18, 80, 52);
	_btnVeteran = new TextButton(160, 18, 80, 72);
	_btnGenius = new TextButton(160, 18, 80, 92);
	_btnSuperhuman = new TextButton(160, 18, 80, 112);
	_btnIronman = new ToggleTextButton(78, 18, 80, 138);
	_btnOk = new TextButton(78, 16, 80, 164);
	_btnCancel = new TextButton(78, 16, 162, 164);
	_txtTitle = new Text(192, 9, 64, 20);
	_txtIronman = new Text(90, 24, 162, 135);

	switch (getGame()->getMod()->getStartingDifficulty())
	{
	case 0:
		_difficulty = _btnBeginner;
		break;
	case 1:
		_difficulty = _btnExperienced;
		break;
	case 2:
		_difficulty = _btnVeteran;
		break;
	case 3:
		_difficulty = _btnGenius;
		break;
	case 4:
		_difficulty = _btnSuperhuman;
		break;
	default:
		_difficulty = _btnBeginner;
		break;
	}

	// Set palette
	setInterface("newGameMenu");

	add(_window, "window", "newGameMenu");
	add(_btnBeginner, "button", "newGameMenu");
	add(_btnExperienced, "button", "newGameMenu");
	add(_btnVeteran, "button", "newGameMenu");
	add(_btnGenius, "button", "newGameMenu");
	add(_btnSuperhuman, "button", "newGameMenu");
	add(_btnIronman, "ironman", "newGameMenu");
	add(_btnOk, "button", "newGameMenu");
	add(_btnCancel, "button", "newGameMenu");
	add(_txtTitle, "text", "newGameMenu");
	add(_txtIronman, "ironman", "newGameMenu");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "newGameMenu");

	_btnBeginner->setText(tr("STR_1_BEGINNER"));
	_btnBeginner->setGroup(&_difficulty);

	_btnExperienced->setText(tr("STR_2_EXPERIENCED"));
	_btnExperienced->setGroup(&_difficulty);

	_btnVeteran->setText(tr("STR_3_VETERAN"));
	_btnVeteran->setGroup(&_difficulty);

	_btnGenius->setText(tr("STR_4_GENIUS"));
	_btnGenius->setGroup(&_difficulty);

	_btnSuperhuman->setText(tr("STR_5_SUPERHUMAN"));
	_btnSuperhuman->setGroup(&_difficulty);

	_btnIronman->setText(tr("STR_IRONMAN"));

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&NewGameState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&NewGameState::btnOkClick, Options::keyOk);

	_btnCancel->setText(tr("STR_CANCEL"));
	_btnCancel->onMouseClick((ActionHandler)&NewGameState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&NewGameState::btnCancelClick, Options::keyCancel);

	_txtTitle->setAlign(TextHAlign::ALIGN_CENTER);
	_txtTitle->setText(tr("STR_SELECT_DIFFICULTY_LEVEL"));

	_txtIronman->setWordWrap(true);
	_txtIronman->setVerticalAlign(TextVAlign::ALIGN_MIDDLE);
	_txtIronman->setText(tr("STR_IRONMAN_DESC"));
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void NewGameState::btnOkClick(Action *)
{
	GameDifficulty diff = DIFF_BEGINNER;
	if (_difficulty == _btnBeginner)
	{
		diff = DIFF_BEGINNER;
	}
	else if (_difficulty == _btnExperienced)
	{
		diff = DIFF_EXPERIENCED;
	}
	else if (_difficulty == _btnVeteran)
	{
		diff = DIFF_VETERAN;
	}
	else if (_difficulty == _btnGenius)
	{
		diff = DIFF_GENIUS;
	}
	else if (_difficulty == _btnSuperhuman)
	{
		diff = DIFF_SUPERHUMAN;
	}

	// Reset touch flags
	getGame()->resetTouchButtonFlags();

	SavedGame *save = getGame()->getMod()->newSave(diff);
	save->setDifficulty(diff);
	save->setIronman(_btnIronman->getPressed());
	getGame()->setSavedGame(save);

	GeoscapeState *gs = new GeoscapeState;
	getGame()->setState(gs);
	gs->init();

	entt::handle newBaseHandle = getRegistry().front<Base, GeoPosition>(); // a base is created in newSave
	if (newBaseHandle.get<Base>().getMarker() != -1)
	{
		// location known already
		newBaseHandle.get<Base>().calculateServices(save);

		// center and rotate 35 degrees down (to see the base location while typoing its name)
		gs->getGlobe()->center(newBaseHandle.get<GeoPosition>() + GeoPosition{0, 0.61});

		if (newBaseHandle.get<Base>().getName().empty())
		{
			// fixed location, custom name
			getGame()->pushState(new BaseNameState(newBaseHandle, gs->getGlobe(), true, true));
		}
		else if (Options::customInitialBase)
		{
			// fixed location, fixed name
			getGame()->pushState(new PlaceLiftState(newBaseHandle, gs->getGlobe(), true));
		}
	}
	else
	{
		// custom location, custom name
		getGame()->pushState(new BuildNewBaseState(newBaseHandle, gs->getGlobe(), true));
	}
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void NewGameState::btnCancelClick(Action *)
{
	getGame()->setSavedGame(0);
	getGame()->popState();
}

}
