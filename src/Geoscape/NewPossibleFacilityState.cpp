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
#include "NewPossibleFacilityState.h"
#include "../Basescape/BasescapeState.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Interface/Window.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleBaseFacility.h"
#include "../Savegame/Base.h"
#include "../Entity/Interface/Interface.h"

namespace OpenXcom
{
/**
 * Initializes all the elements in the NewPossibleFacility screen.
 * @param base Pointer to the base to get info from.
 * @param globe Pointer to the globe.
 * @param possibilities List of newly possible base facilities to build
 */
NewPossibleFacilityState::NewPossibleFacilityState(Base* base, Globe* globe, const std::vector<RuleBaseFacility*>& possibilities)
	: State("NewPossibleFacilityState", false), _base(base), _globe(globe)
{
	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();

	// Create objects
	_window = factory.createWindow("windowName", this, 288, 180, 16, 10);
	_btnOk = new TextButton(160, 14, 80, 149);
	_btnOpen = new TextButton(160, 14, 80, 165);
	_txtTitle = new Text(288, 40, 16, 20);
	_lstPossibilities = new TextList(250, 80, 35, 50);
	_txtCaveat = new Text(250, 16, 35, 131);

	// Set palette
	setInterface("geoNewFacility");

	add(_window, "window", "geoNewFacility");
	add(_btnOk, "button", "geoNewFacility");
	add(_btnOpen, "button", "geoNewFacility");
	add(_txtTitle, "text1", "geoNewFacility");
	add(_lstPossibilities, "text2", "geoNewFacility");
	add(_txtCaveat, "text1", "geoNewFacility");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "geoNewFacility");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&NewPossibleFacilityState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&NewPossibleFacilityState::btnOkClick, Options::keyCancel);
	_btnOpen->setText(tr("STR_BASES"));
	_btnOpen->onMouseClick((ActionHandler)&NewPossibleFacilityState::btnOpenClick);
	_btnOpen->onKeyboardPress((ActionHandler)&NewPossibleFacilityState::btnOpenClick, Options::keyOk);
	_txtTitle->setBig();
	_txtTitle->setAlign(TextHAlign::ALIGN_CENTER);
	_txtTitle->setText(tr("STR_WE_CAN_NOW_BUILD"));

	// Caveat
	{
		RuleBaseFacilityFunctions requiredServices;
		for (const auto* facRule : possibilities)
		{
			requiredServices |= facRule->getRequireBaseFunc();
		}
		std::ostringstream ss;
		int i = 0;
		for (const std::string& serviceName : getGame()->getMod()->getBaseFunctionNames(requiredServices))
		{
			if (i > 0)
				ss << ", ";
			ss << tr(serviceName);
			i++;
		}
		std::string argument = ss.str();

		_txtCaveat->setAlign(TextHAlign::ALIGN_CENTER);
		_txtCaveat->setText(tr("STR_REQUIRED_BASE_SERVICES").arg(argument));
		_txtCaveat->setVisible(requiredServices.any());
	}

	_lstPossibilities->setColumns(1, 250);
	_lstPossibilities->setBig();
	_lstPossibilities->setAlign(TextHAlign::ALIGN_CENTER);
	_lstPossibilities->setScrolling(true, 0);
	for (const auto* facRule : possibilities)
	{
		_lstPossibilities->addRow (1, tr(facRule->getType()).c_str());
	}
}

/**
 * Closes the screen.
 * @param action Pointer to an action.
 */
void NewPossibleFacilityState::btnOkClick(Action *)
{
	getGame()->popState();
}

/**
 * Opens the BaseView.
 * @param action Pointer to an action.
 */
void NewPossibleFacilityState::btnOpenClick(Action *)
{
	getGame()->popState();
	getGame()->pushState(new BasescapeState(getRegistry().getService<BasescapeSystem>(), _globe));
}

}
