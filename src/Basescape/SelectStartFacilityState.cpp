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
#include "SelectStartFacilityState.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleBaseFacility.h"
#include "../Savegame/Base.h"
#include "../Savegame/BaseFacility.h"
#include "../Savegame/SavedGame.h"
#include "../Engine/Options.h"
#include "PlaceStartFacilityState.h"
#include "PlaceLiftState.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Build Facilities window.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param state Pointer to the base state to refresh.
 * @param globe Pointer to the globe to refresh.
 */
SelectStartFacilityState::SelectStartFacilityState(entt::entity newBaseId, State *state, Globe *globe)
	: BuildFacilitiesState(newBaseId, state), _globe(globe)
{
	_facilities = getGame()->getMod()->getCustomBaseFacilities(getGame()->getSavedGame()->getDifficulty());

	_btnOk->setText(tr("STR_RESET"));
	_btnOk->onMouseClick((ActionHandler)&SelectStartFacilityState::btnOkClick);
	_btnOk->onKeyboardPress(0, Options::keyCancel);

	_lstFacilities->onMouseClick((ActionHandler)&SelectStartFacilityState::lstFacilitiesClick);
}

/**
 * Populates the build list from the current "available" facilities.
 */
void SelectStartFacilityState::populateBuildList()
{
	_lstFacilities->clearList();
	for (const auto* rule : _facilities)
	{
		_lstFacilities->addRow(1, tr(rule->getType()).c_str());
	}
}

/**
 * Resets the base building.
 * @param action Pointer to an action.
 */
void SelectStartFacilityState::btnOkClick(Action *)
{
	Base& base = getGame()->getSavedGame()->getRegistry().get<Base>(_baseId);
	for (BaseFacility* fac : base.getFacilities())
	{
		delete fac;
	}
	base.getFacilities().clear();
	getGame()->popState();
	getGame()->popState();
	getGame()->pushState(new PlaceLiftState(_baseId, _globe, true));
}

/**
 * Places the selected facility.
 * @param action Pointer to an action.
 */
void SelectStartFacilityState::lstFacilitiesClick(Action *)
{
	getGame()->pushState(new PlaceStartFacilityState(_baseId, this, _facilities[_lstFacilities->getSelectedRow()]));
}

/**
 * Callback from PlaceStartFacilityState.
 * Removes placed facility from the list.
 */
void SelectStartFacilityState::facilityBuilt()
{
	_facilities.erase(_facilities.begin() + _lstFacilities->getSelectedRow());
	if (_facilities.empty())
	{
		getGame()->popState();
		getGame()->popState(); // return to geoscape, force timer to start.
	}
	else
	{
		populateBuildList();
	}
}

}
