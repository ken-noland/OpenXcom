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
#include "AbandonGameState.h"
#include "../Engine/Sound.h"
#include "../Engine/Game.h"
#include "../Engine/Options.h"
#include "../Mod/Mod.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "MainMenuState.h"
#include "../Engine/Screen.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/SavedBattleGame.h"
#include "SaveGameState.h"
#include "../Entity/Interface/Interface.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Abandon Game screen.
 * @param game Pointer to the core game.
 * @param origin Game section that originated this state.
 */
AbandonGameState::AbandonGameState(OptionsOrigin origin)
	: State("AbandonGameState", false), _origin(origin)
{
	int x;
	if (_origin == OPT_GEOSCAPE)
	{
		x = 20;
	}
	else
	{
		x = 52;
	}

	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();

	// Create objects
	_window = factory.createWindow("windowName", this, 216, 160, x, 20, WindowPopup::POPUP_BOTH);
	_btnYes = new TextButton(50, 20, x+18, 140);
	_btnNo = new TextButton(50, 20, x+148, 140);
	_txtTitle = new Text(206, 17, x+5, 70);

	// Set palette
	setInterface("geoscape", false, getGame()->getSavedGame() ? getGame()->getSavedGame()->getSavedBattle() : 0);

	add(_window, "genericWindow", "geoscape");
	add(_btnYes, "genericButton2", "geoscape");
	add(_btnNo, "genericButton2", "geoscape");
	add(_txtTitle, "genericText", "geoscape");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "geoscape");

	_btnYes->setText(tr("STR_YES"));
	_btnYes->onMouseClick((ActionHandler)&AbandonGameState::btnYesClick);
	_btnYes->onKeyboardPress((ActionHandler)&AbandonGameState::btnYesClick, Options::keyOk);

	_btnNo->setText(tr("STR_NO"));
	_btnNo->onMouseClick((ActionHandler)&AbandonGameState::btnNoClick);
	_btnNo->onKeyboardPress((ActionHandler)&AbandonGameState::btnNoClick, Options::keyCancel);

	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();
	_txtTitle->setText(tr("STR_ABANDON_GAME_QUESTION"));

	if (_origin == OPT_BATTLESCAPE)
	{
		applyBattlescapeTheme("geoscape");
	}
}

/**
 *
 */
AbandonGameState::~AbandonGameState()
{

}

/**
 * Goes back to the Main Menu.
 * @param action Pointer to an action.
 */
void AbandonGameState::btnYesClick(Action *)
{
	// Reset touch flags
	getGame()->resetTouchButtonFlags();

	if (_origin == OPT_BATTLESCAPE && getGame()->getSavedGame()->getSavedBattle()->getAmbientSound() != Mod::NO_SOUND)
		getGame()->getMod()->getSoundByDepth(0, getGame()->getSavedGame()->getSavedBattle()->getAmbientSound())->stopLoop();
	if (!getGame()->getSavedGame()->isIronman())
	{
		Screen::updateScale(Options::geoscapeScale, Options::baseXGeoscape, Options::baseYGeoscape, true);
		getGame()->getScreen()->resetDisplay(false);

		getGame()->setState(new MainMenuState);
		getGame()->setSavedGame(0);
	}
	else
	{
		getGame()->pushState(new SaveGameState(OPT_GEOSCAPE, SAVE_IRONMAN_END, _palette));
	}
}

/**
 * Closes the window.
 * @param action Pointer to an action.
 */
void AbandonGameState::btnNoClick(Action *)
{
	getGame()->popState();
}

}
