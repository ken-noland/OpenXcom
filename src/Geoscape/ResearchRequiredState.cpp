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
#include "ResearchRequiredState.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Mod/Mod.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Mod/RuleItem.h"
#include "../Engine/Options.h"
#include "../Entity/Interface/Interface.h"

namespace OpenXcom
{
/**
 * Initializes all the elements in the Research Required screen.
 * @param game Pointer to the core game.
 * @param item Pointer to the researched weapon.
 */
ResearchRequiredState::ResearchRequiredState(RuleItem *item) : State("ResearchRequiredState", false)
{
	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();

	// Create objects
	_window = factory.createWindow("windowName", this, 288, 180, 16, 10);
	_btnOk = new TextButton(160, 18, 80, 150);
	_txtTitle = new Text(288, 80, 16, 50);

	// Set palette
	setInterface("geoResearchRequired");

	add(_window, "window", "geoResearchRequired");
	add(_btnOk, "button", "geoResearchRequired");
	add(_txtTitle, "text1", "geoResearchRequired");

	centerAllSurfaces();

	std::string weapon = item->getType();
	std::string clip = item->getPrimaryCompatibleAmmo()->front()->getType();

	// Set up objects
	setWindowBackground(_window, "geoResearchRequired");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&ResearchRequiredState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&ResearchRequiredState::btnOkClick, Options::keyCancel);
	_btnOk->onKeyboardPress((ActionHandler)&ResearchRequiredState::btnOkClick, Options::keyOk);

	_txtTitle->setBig();
	_txtTitle->setAlign(TextHAlign::ALIGN_CENTER);
	_txtTitle->setVerticalAlign(TextVAlign::ALIGN_MIDDLE);
	_txtTitle->setText(tr("STR_YOU_NEED_TO_RESEARCH_ITEM_TO_PRODUCE_ITEM")
					   .arg(tr(clip))
					   .arg(tr(weapon)));
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void ResearchRequiredState::btnOkClick(Action *)
{
	getGame()->popState();
}

}
