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
#include "AlienBaseState.h"
#include "GeoscapeState.h"
#include "Globe.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleCountry.h"
#include "../Mod/RuleRegion.h"
#include "../Savegame/AlienBase.h"
#include "../Savegame/AreaSystem.h"
#include "../Savegame/Country.h"
#include "../Savegame/Region.h"
#include "../Savegame/SavedGame.h"
#include "../Entity/Interface/Interface.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Aliens Base discovered window.
 * @param game Pointer to the core game.
 * @param base Pointer to the alien base to get info from.
 * @param state Pointer to the Geoscape.
 */
AlienBaseState::AlienBaseState(AlienBase *base, GeoscapeState *state) : State("AlienBaseState", true), _state(state), _base(base)
{
	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();

	// Create objects
	_window = factory.createWindow("windowName", this, 320, 200, 0, 0);
	_btnOk = new TextButton(50, 12, 135, 180);
	_txtTitle = new Text(308, 60, 6, 60);

	setInterface("alienBase");

	add(_window, "window", "alienBase");
	add(_btnOk, "text", "alienBase");
	add(_txtTitle, "button", "alienBase");

	centerAllSurfaces();


	// Set up objects
	setWindowBackground(_window, "alienBase");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&AlienBaseState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&AlienBaseState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&AlienBaseState::btnOkClick, Options::keyCancel);

	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();
	_txtTitle->setWordWrap(true);

	// Check location of base
	std::string countryName, regionName;
	if (const Country* country = AreaSystem::locateValue<Country>(*base))
	{
		countryName = tr(country->getRules()->getType());
	}
	if (const Region* region = AreaSystem::locateValue<Region>(*base))
	{
		regionName = tr(region->getRules()->getType());
	}

	std::string location = !countryName.empty() ? tr("STR_COUNTRIES_COMMA").arg(countryName).arg(regionName)
						 : regionName.empty()   ? static_cast<LocalizedText>(regionName)
						 : tr("STR_UNKNOWN");
	_txtTitle->setText(tr("STR_XCOM_AGENTS_HAVE_LOCATED_AN_ALIEN_BASE_IN_REGION").arg(location));
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void AlienBaseState::btnOkClick(Action *)
{
	_state->timerReset();
	_state->getGlobe()->center(_base->getLongitude(), _base->getLatitude());
	getGame()->popState();
}

}
