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
#include "CraftPatrolState.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Savegame/Craft.h"
#include "../Savegame/Target.h"
#include "GeoscapeCraftState.h"
#include "../Engine/Options.h"
#include "../Entity/Interface/Interface.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Craft Patrol window.
 * @param game Pointer to the core game.
 * @param craft Pointer to the craft to display.
 * @param globe Pointer to the Geoscape globe.
 */
CraftPatrolState::CraftPatrolState(Craft* craft, Globe* globe) : State("CraftPatrolState", false), _craft(craft), _globe(globe)
{
	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();

	// Create objects
	_window = factory.createWindow("windowName", this, 224, 168, 16, 16, WindowPopup::POPUP_BOTH);
	_btnOk = new TextButton(140, 12, 58, 144);
	_btnRedirect = new TextButton(140, 12, 58, 160);
	_txtDestination = new Text(224, 64, 16, 48);
	_txtPatrolling = new Text(224, 17, 16, 120);

	// Set palette
	setInterface("craftPatrol");

	add(_window, "window", "craftPatrol");
	add(_btnOk, "button", "craftPatrol");
	add(_btnRedirect, "button", "craftPatrol");
	add(_txtDestination, "text1", "craftPatrol");
	add(_txtPatrolling, "text1", "craftPatrol");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "craftPatrol");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&CraftPatrolState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&CraftPatrolState::btnOkClick, Options::keyCancel);

	_btnRedirect->setText(tr("STR_REDIRECT_CRAFT"));
	_btnRedirect->onMouseClick((ActionHandler)&CraftPatrolState::btnRedirectClick);
	_btnRedirect->onKeyboardPress((ActionHandler)&CraftPatrolState::btnRedirectClick, Options::keyOk);

	_txtDestination->setBig();
	_txtDestination->setAlign(TextHAlign::ALIGN_CENTER);
	_txtDestination->setWordWrap(true);
	_txtDestination->setText(tr("STR_CRAFT_HAS_REACHED_DESTINATION")
							 .arg(_craft->getName(getGame()->getLanguage()))
							 .arg(_craft->getDestination()->getName(getGame()->getLanguage())));

	_txtPatrolling->setBig();
	_txtPatrolling->setAlign(TextHAlign::ALIGN_CENTER);
	_txtPatrolling->setText(tr("STR_NOW_PATROLLING"));
}

/**
 *
 */
CraftPatrolState::~CraftPatrolState()
{

}

/**
 * Closes the window.
 * @param action Pointer to an action.
 */
void CraftPatrolState::btnOkClick(Action *)
{
	getGame()->popState();
}

/**
 * Opens up the Craft window.
 * @param action Pointer to an action.
 */
void CraftPatrolState::btnRedirectClick(Action *)
{
	getGame()->popState();
	getGame()->pushState(new GeoscapeCraftState(_craft, _globe, 0, false));
}

}
