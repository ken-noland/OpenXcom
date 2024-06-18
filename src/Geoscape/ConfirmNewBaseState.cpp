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
#include "BaseNameState.h"
#include "ConfirmNewBaseState.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/Unicode.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Menu/ErrorMessageState.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleInterface.h"
#include "../Mod/RuleRegion.h"
#include "../Savegame/AreaSystem.h"
#include "../Savegame/Base.h"
#include "../Savegame/Region.h"
#include "../Savegame/SavedGame.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Confirm New Base window.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to place.
 * @param globe Pointer to the Geoscape globe.
 */
ConfirmNewBaseState::ConfirmNewBaseState(entt::entity newBaseId, Globe *globe)
	: _newBaseId(newBaseId), _globe(globe)
{
	_screen = false;

	// Create objects
	_window = new Window(this, 224, 72, 16, 64);
	_btnOk = new TextButton(54, 12, 68, 104);
	_btnCancel = new TextButton(54, 12, 138, 104);
	_txtCost = new Text(120, 9, 68, 80);
	_txtArea = new Text(120, 9, 68, 90);

	// Set palette
	setInterface("geoscape");

	add(_window, "genericWindow", "geoscape");
	add(_btnOk, "genericButton2", "geoscape");
	add(_btnCancel, "genericButton2", "geoscape");
	add(_txtCost, "genericText", "geoscape");
	add(_txtArea, "genericText", "geoscape");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "geoscape");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&ConfirmNewBaseState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&ConfirmNewBaseState::btnOkClick, Options::keyOk);

	_btnCancel->setText(tr("STR_CANCEL_UC"));
	_btnCancel->onMouseClick((ActionHandler)&ConfirmNewBaseState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&ConfirmNewBaseState::btnCancelClick, Options::keyCancel);

	std::string area;
	entt::registry& registry = getGame()->getSavedGame()->getRegistry();
	entt::entity regionId = AreaSystem::locate<Region>(registry, registry.get<Base>(_newBaseId));
	if (regionId != entt::null)
	{
		const RuleRegion& ruleRegion = *registry.get<Region>(regionId).getRules();
		_cost = ruleRegion.getBaseCost();
		area = tr(ruleRegion.getType());
	}

	_txtCost->setText(tr("STR_COST_").arg(Unicode::formatFunding(_cost)));

	_txtArea->setText(tr("STR_AREA_").arg(area));
}

/**
 * Go to the Place Access Lift screen.
 * @param action Pointer to an action.
 */
void ConfirmNewBaseState::btnOkClick(Action *) const
{
	if (getGame()->getSavedGame()->getFunds() >= _cost)
	{
		getGame()->getSavedGame()->setFunds(getGame()->getSavedGame()->getFunds() - _cost);
		getGame()->pushState(new BaseNameState(_newBaseId, _globe, false, false));
	}
	else
	{
		getGame()->pushState(new ErrorMessageState(tr("STR_NOT_ENOUGH_MONEY"), _palette, getGame()->getMod()->getInterface("geoscape")->getElement("genericWindow")->color, "BACK01.SCR", getGame()->getMod()->getInterface("geoscape")->getElement("palette")->color));
	}
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void ConfirmNewBaseState::btnCancelClick(Action *)
{
	_globe->onMouseOver(0);
	getGame()->popState();
}

}
