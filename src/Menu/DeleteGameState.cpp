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
#include "DeleteGameState.h"
#include "../Engine/CrossPlatform.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/Text.h"
#include "../Interface/Window.h"
#include "../Interface/TextButton.h"
#include "../Mod/Mod.h"
#include "../Engine/Options.h"
#include "ErrorMessageState.h"
#include "../Savegame/SavedGame.h"
#include "../Mod/RuleInterface.h"
#include "../Entity/Interface/Interface.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Confirmation screen.
 * @param game Pointer to the core game.
 * @param origin Game section that originated this state.
 * @param save Name of the save file to delete.
 */
DeleteGameState::DeleteGameState(OptionsOrigin origin, const std::string &save)
	: State("DeleteGameState", false), _origin(origin)
{
	_filename = Options::getMasterUserFolder() + save;

	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();

	// Create objects
	_window = factory.createWindow("windowName", this, 256, 100, 32, 50, WindowPopup::POPUP_BOTH);
	_btnYes = new TextButton(60, 18, 60, 122);
	_btnNo = new TextButton(60, 18, 200, 122);
	_txtMessage = new Text(246, 32, 37, 70);

	// Set palette
	setInterface("saveMenus", false, getGame()->getSavedGame() ? getGame()->getSavedGame()->getSavedBattle() : 0);

	add(_window, "confirmDelete", "saveMenus");
	add(_btnYes, "confirmDelete", "saveMenus");
	add(_btnNo, "confirmDelete", "saveMenus");
	add(_txtMessage, "confirmDelete", "saveMenus");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "saveMenus");

	_btnYes->setText(tr("STR_YES"));
	_btnYes->onMouseClick((ActionHandler)&DeleteGameState::btnYesClick);
	_btnYes->onKeyboardPress((ActionHandler)&DeleteGameState::btnYesClick, Options::keyOk);

	_btnNo->setText(tr("STR_NO"));
	_btnNo->onMouseClick((ActionHandler)&DeleteGameState::btnNoClick);
	_btnNo->onKeyboardPress((ActionHandler)&DeleteGameState::btnNoClick, Options::keyCancel);

	_txtMessage->setAlign(TextHAlign::ALIGN_CENTER);
	_txtMessage->setBig();
	_txtMessage->setWordWrap(true);
	_txtMessage->setText(tr("STR_IS_IT_OK_TO_DELETE_THE_SAVED_GAME"));

	if (_origin == OPT_BATTLESCAPE)
	{
		applyBattlescapeTheme("saveMenus");
	}
}

/**
 *
 */
DeleteGameState::~DeleteGameState()
{

}

void DeleteGameState::btnNoClick(Action *)
{
	getGame()->popState();
}

void DeleteGameState::btnYesClick(Action *)
{
	getGame()->popState();
	if (!CrossPlatform::deleteFile(_filename))
	{
		std::string error = tr("STR_DELETE_UNSUCCESSFUL");
		if (_origin != OPT_BATTLESCAPE)
			getGame()->pushState(new ErrorMessageState(error, _palette, getGame()->getMod()->getInterface("errorMessages")->getElement("geoscapeColor")->color, "BACK01.SCR", getGame()->getMod()->getInterface("errorMessages")->getElement("geoscapePalette")->color));
		else
			getGame()->pushState(new ErrorMessageState(error, _palette, getGame()->getMod()->getInterface("errorMessages")->getElement("battlescapeColor")->color, "TAC00.SCR", getGame()->getMod()->getInterface("errorMessages")->getElement("battlescapePalette")->color));
	}
}

}
