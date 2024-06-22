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
#include "PlaceFacilityState.h"
#include "../Engine/Game.h"
#include "../Engine/Sound.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "BaseView.h"
#include "../Savegame/Base.h"
#include "../Savegame/BaseFacility.h"
#include "../Savegame/ItemContainer.h"
#include "../Mod/RuleBaseFacility.h"
#include "../Savegame/SavedGame.h"
#include "../Menu/ErrorMessageState.h"
#include "../Engine/Options.h"
#include "../Engine/Unicode.h"
#include "../Mod/RuleInterface.h"
#include "../Entity/Interface/Interface.h"
#include <climits>

namespace OpenXcom
{

/**
 * Initializes all the elements in the Place Facility window.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param rule Pointer to the facility ruleset to build.
 */
PlaceFacilityState::PlaceFacilityState(entt::entity baseId, const RuleBaseFacility *rule, BaseFacility *origFac)
	: State("PlaceFacilityState", false), _baseHandle(baseId), _rule(rule), _origFac(origFac)
{
	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();

	// Create objects
	_window = factory.createWindow("placeFacility", this, 128, 160, 192, 40);
	_view = new BaseView(192, 192, 0, 8);
	_btnCancel = new TextButton(112, 16, 200, 176);
	_txtFacility = new Text(110, 9, 202, 50);
	_txtCost = new Text(110, 9, 202, 62);
	_numCost = new Text(110, 17, 202, 70);
	_numResources = new Text(110, 25, 202, 87);
	const size_t resourceTextOffset = 9*std::min((size_t)3, (_origFac == nullptr ? _rule->getBuildCostItems().size() : 0));
	_txtTime = new Text(110, 9, 202, static_cast<int>(90 + resourceTextOffset));
	_numTime = new Text(110, 17, 202, static_cast<int>(98 + resourceTextOffset));
	_txtMaintenance = new Text(110, 9, 202, static_cast<int>(118+resourceTextOffset));
	_numMaintenance = new Text(110, 17, 202, static_cast<int>(126+resourceTextOffset));

	// Set palette
	setInterface("placeFacility");

	add(_window, "window", "placeFacility");
	add(_view, "baseView", "basescape");
	add(_btnCancel, "button", "placeFacility");
	add(_txtFacility, "text", "placeFacility");
	add(_txtCost, "text", "placeFacility");
	add(_numCost, "numbers", "placeFacility");
	add(_numResources, "numbers", "placeFacility");
	add(_txtTime, "text", "placeFacility");
	add(_numTime, "numbers", "placeFacility");
	add(_txtMaintenance, "text", "placeFacility");
	add(_numMaintenance, "numbers", "placeFacility");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "placeFacility");

	auto* itf = getGame()->getMod()->getInterface("basescape")->getElement("trafficLights");
	if (itf)
	{
		_view->setOtherColors(itf->color, itf->color2, itf->border, !itf->TFTDMode);
	}
	Base& base = getRegistry().raw().get<Base>(_baseHandle);
	_view->setTexture(getGame()->getMod()->getSurfaceSet("BASEBITS.PCK"));
	_view->setBase(&base);
	_view->setSelectable(rule->getSizeX(), rule->getSizeY());
	_view->onMouseClick((ActionHandler)&PlaceFacilityState::viewClick);

	_btnCancel->setText(tr("STR_CANCEL"));
	_btnCancel->onMouseClick((ActionHandler)&PlaceFacilityState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&PlaceFacilityState::btnCancelClick, Options::keyCancel);

	_txtFacility->setText(tr(_rule->getType()));

	_txtCost->setText(tr("STR_COST_UC"));

	_numCost->setBig();
	_numCost->setText(Unicode::formatFunding(_origFac != nullptr ? 0 : _rule->getBuildCost()));

	if (_origFac == nullptr && !_rule->getBuildCostItems().empty())
	{
		std::ostringstream ss;

		// Currently, the text box will only fit three lines of items.
		// But I'm going to add everything to the list anyway.
		for (auto& item : _rule->getBuildCostItems())
		{
			// Note: `item` is of the form (item name, (cost number, refund number))
			size_t max = 19;
			if (item.second.first > 9) --max;
			if (item.second.first > 99) --max;
			std::string name = tr(item.first);
			if (Unicode::codePointLengthUTF8(name) > max)
			{
				name = Unicode::codePointSubstrUTF8(name, 0, max);
			}

			ss << name << ": " << item.second.first << std::endl;
		}
		_numResources->setText(ss.str());
	}

	_txtTime->setText(tr("STR_CONSTRUCTION_TIME_UC"));

	_numTime->setBig();
	_numTime->setText(tr("STR_DAY", _origFac != 0 ? 0 : _rule->getBuildTime()));

	_txtMaintenance->setText(tr("STR_MAINTENANCE_UC"));

	_numMaintenance->setBig();
	_numMaintenance->setText(Unicode::formatFunding(_rule->getMonthlyCost()));
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void PlaceFacilityState::btnCancelClick(Action *)
{
	getGame()->popState();
}

/**
 * Processes clicking on facilities.
 * @param action Pointer to an action.
 */
void PlaceFacilityState::viewClick(Action *)
{
	if (_origFac != 0)
	{
		// EXPERIMENTAL!: just moving an existing facility
		if (_view->getGridX() == _origFac->getX() && _view->getGridY() == _origFac->getY())
		{
			// unchanged location -> no message, no cost.
			getGame()->popState();
		}
		else if (_view->getPlacementError(_rule, _origFac))
		{
			getGame()->pushState(new ErrorMessageState(tr("STR_CANNOT_BUILD_HERE"), _palette, getGame()->getMod()->getInterface("placeFacility")->getElement("errorMessage")->color, "BACK01.SCR", getGame()->getMod()->getInterface("placeFacility")->getElement("errorPalette")->color));
		}
		else
		{
			_origFac->setX(_view->getGridX());
			_origFac->setY(_view->getGridY());
			if (Options::allowBuildingQueue)
			{
				// first reset (maybe the moved facility is not queued anymore)
				if (abs(_origFac->getBuildTime()) > _rule->getBuildTime()) _origFac->setBuildTime(_rule->getBuildTime());
				// if it is still in the queue though, recalc
				if (_origFac->getBuildTime() > 0 && _view->isQueuedBuilding(_rule)) _origFac->setBuildTime(INT_MAX);
				_view->reCalcQueuedBuildings();
			}
			getGame()->popState();
		}
	}
	else
	{
		// placing a brand new facility
		BasePlacementErrors placementErrorCode = _view->getPlacementError(_rule);
		if (placementErrorCode)
		{
			int errorColor1 = getGame()->getMod()->getInterface("placeFacility")->getElement("errorMessage")->color;
			int errorColor2 = getGame()->getMod()->getInterface("placeFacility")->getElement("errorPalette")->color;
			switch (placementErrorCode)
			{
				case BPE_Used_Stores:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_IN_USE_STORAGE"), _palette, errorColor1, "BACK01.SCR", errorColor2));
					break;
				case BPE_Used_Quarters:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_IN_USE_QUARTERS"), _palette, errorColor1, "BACK01.SCR", errorColor2));
					break;
				case BPE_Used_Laboratories:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_IN_USE_LABORATORIES"), _palette, errorColor1, "BACK01.SCR", errorColor2));
					break;
				case BPE_Used_Workshops:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_IN_USE_WORKSHOPS"), _palette, errorColor1, "BACK01.SCR", errorColor2));
					break;
				case BPE_Used_Hangars:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_IN_USE_HANGARS"), _palette, errorColor1, "BACK01.SCR", errorColor2));
					break;
				case BPE_Used_PsiLabs:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_IN_USE_PSI_LABS"), _palette, errorColor1, "BACK01.SCR", errorColor2));
					break;
				case BPE_Used_Gyms:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_IN_USE_GYMS"), _palette, errorColor1, "BACK01.SCR", errorColor2));
					break;
				case BPE_Used_AlienContainment:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_IN_USE_PRISONS"), _palette, errorColor1, "BACK01.SCR", errorColor2));
					break;
				case BPE_NotConnected:
					getGame()->pushState(new ErrorMessageState(tr("STR_CANNOT_BUILD_HERE"), _palette, errorColor1, "BACK01.SCR", errorColor2));
					break;
				case BPE_Used:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_IN_USE"), _palette, errorColor1, "BACK01.SCR", errorColor2));
					break;
				case BPE_Upgrading:
					getGame()->pushState(new ErrorMessageState(tr("STR_CANNOT_UPGRADE_FACILITY_ALREADY_UPGRADING"), _palette, errorColor1, "BACK01.SCR", errorColor2));
					break;
				case BPE_UpgradeSizeMismatch:
					getGame()->pushState(new ErrorMessageState(tr("STR_CANNOT_UPGRADE_FACILITY_WRONG_SIZE"), _palette, errorColor1, "BACK13.SCR", errorColor2));
					break;
				case BPE_UpgradeRequireSpecific:
					getGame()->pushState(new ErrorMessageState(tr("STR_CANNOT_UPGRADE_FACILITY_WRONG_TYPE"), _palette, errorColor1, "BACK13.SCR", errorColor2));
					break;
				case BPE_UpgradeDisallowed:
					getGame()->pushState(new ErrorMessageState(tr("STR_CANNOT_UPGRADE_FACILITY_DISALLOWED"), _palette, errorColor1, "BACK13.SCR", errorColor2));
					break;
				case BPE_Queue:
					getGame()->pushState(new ErrorMessageState(tr("STR_CANNOT_BUILD_QUEUE_OFF"), _palette, errorColor1, "BACK13.SCR", errorColor2));
					break;
				case BPE_ForbiddenByOther:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_FORBIDDEN_BY_OTHER"), _palette, errorColor1, "BACK01.SCR", errorColor2));
					break;
				case BPE_ForbiddenByThis:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_OTHER_FORBIDDEN_BY_THIS"), _palette, errorColor1, "BACK01.SCR", errorColor2));
					break;
				default:
					getGame()->pushState(new ErrorMessageState(tr("STR_CANNOT_BUILD_HERE"), _palette, errorColor1, "BACK01.SCR", errorColor2));
					break;
			}
		}
		else if (getGame()->getSavedGame()->getFunds() < _rule->getBuildCost())
		{
			getGame()->popState();
			getGame()->pushState(new ErrorMessageState(tr("STR_NOT_ENOUGH_MONEY"), _palette, getGame()->getMod()->getInterface("placeFacility")->getElement("errorMessage")->color, "BACK01.SCR", getGame()->getMod()->getInterface("placeFacility")->getElement("errorPalette")->color));
		}
		else
		{
			Base& base = getRegistry().raw().get<Base>(_baseHandle);
			for (const auto& item: _rule->getBuildCostItems())
			{
				int needed = item.second.first - base.getStorageItems()->getItem(item.first);
				if (needed > 0)
				{
					getGame()->popState();
					getGame()->pushState(new ErrorMessageState(tr("STR_NOT_ENOUGH_ITEMS").arg(tr(item.first)).arg(needed), _palette, getGame()->getMod()->getInterface("placeFacility")->getElement("errorMessage")->color, "BACK01.SCR", getGame()->getMod()->getInterface("placeFacility")->getElement("errorPalette")->color));
					return;
				}
			}
			// Remove any facilities we're building over
			double reducedBuildTime = 0.0;
			bool buildingOver = false;
			const BaseAreaSubset areaToBuildOver = BaseAreaSubset(_rule->getSizeX(), _rule->getSizeY()).offset(_view->getGridX(), _view->getGridY());
			for (BaseFacility* checkFacility : base.getFacilities() | std::views::reverse)
			{
				if (BaseAreaSubset::intersection(areaToBuildOver, checkFacility->getPlacement()))
				{
					// Get a refund from the facility we're building over
					const std::map<std::string, std::pair<int, int> > &itemCost = checkFacility->getRules()->getBuildCostItems();

					if (checkFacility->getBuildTime() > checkFacility->getRules()->getBuildTime())
					{
						// Give full refund if this is a (not yet started) queued build.
						getGame()->getSavedGame()->setFunds(getGame()->getSavedGame()->getFunds() + checkFacility->getRules()->getBuildCost());
						for (auto& item : itemCost)
						{
							base.getStorageItems()->addItem(getGame()->getMod()->getItem(item.first, true), item.second.first);
						}
					}
					else
					{
						// Give partial refund if this is a started build or a completed facility.
						getGame()->getSavedGame()->setFunds(getGame()->getSavedGame()->getFunds() + checkFacility->getRules()->getRefundValue());
						for (auto& item : itemCost)
						{
							base.getStorageItems()->addItem(getGame()->getMod()->getItem(item.first, true), item.second.second);
						}

						// Reduce the build time of the new facility
						double oldSizeSquared = (checkFacility->getRules()->getSizeX() * checkFacility->getRules()->getSizeY());
						double newSizeSquared = (_rule->getSizeX() * _rule->getSizeY());
						reducedBuildTime += (checkFacility->getRules()->getBuildTime() - checkFacility->getBuildTime()) * oldSizeSquared / newSizeSquared;

						// This only counts as building over something if it wasn't in construction
						if (checkFacility->getBuildTime() == 0)
							buildingOver = true;
					}
					if (checkFacility->getAmmo() > 0)
					{
						// Full refund of loaded ammo
						base.getStorageItems()->addItem(checkFacility->getRules()->getAmmoItem(), checkFacility->getAmmo());
						checkFacility->setAmmo(0);
					}

					// Remove the facility from the base
					auto it = std::ranges::find(base.getFacilities(), checkFacility);
					if (it != base.getFacilities().end())
					{
						base.getFacilities().erase(it);
					}
					delete checkFacility;
				}

			}

			BaseFacility *fac = new BaseFacility(_rule, &base);
			fac->setX(_view->getGridX());
			fac->setY(_view->getGridY());
			fac->setBuildTime(_rule->getBuildTime());
			if (buildingOver)
			{
				fac->setIfHadPreviousFacility(true);
				reducedBuildTime = reducedBuildTime * getGame()->getMod()->getBuildTimeReductionScaling() / 100.0;
				int reducedBuildTimeRounded = (int)std::round(reducedBuildTime);
				fac->setBuildTime(std::max(1, fac->getBuildTime() - reducedBuildTimeRounded));
			}
			base.getFacilities().push_back(fac);
			if (fac->getRules()->getPlaceSound() != Mod::NO_SOUND)
			{
				getGame()->getMod()->getSound("GEO.CAT", fac->getRules()->getPlaceSound())->play();
			}
			if (Options::allowBuildingQueue)
			{
				if (_view->isQueuedBuilding(_rule)) fac->setBuildTime(INT_MAX);
				_view->reCalcQueuedBuildings();
			}
			_view->setBase(&base);
			getGame()->getSavedGame()->setFunds(getGame()->getSavedGame()->getFunds() - _rule->getBuildCost());
			for (const auto& item: _rule->getBuildCostItems())
			{
				base.getStorageItems()->removeItem(item.first, item.second.first);
			}
			getGame()->popState();
		}
	}
}

}
