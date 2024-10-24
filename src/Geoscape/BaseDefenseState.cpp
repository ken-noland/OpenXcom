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
#include "BaseDefenseState.h"
#include "GeoscapeState.h"
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/RNG.h"
#include "../Engine/Sound.h"
#include "../Engine/Timer.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Interface/Window.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleBaseFacility.h"
#include "../Mod/RuleRegion.h"
#include "../Savegame/AlienMission.h"
#include "../Savegame/AreaSystem.h"
#include "../Savegame/Base.h"
#include "../Savegame/BaseFacility.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/Region.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Ufo.h"
#include "../Entity/Interface/Interface.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Base Defense screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base being attacked.
 * @param ufo Pointer to the attacking ufo.
 * @param state Pointer to the Geoscape.
 */
BaseDefenseState::BaseDefenseState(Base* base, Ufo* ufo, GeoscapeState* state)
	: State("BaseDefenseState", true), _state(state)
{
	_base = base;
	_action = BDA_NONE;
	_row = -1;
	_passes = 0;
	_attacks = 0;
	_thinkcycles = 0;
	_ufo = ufo;

	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();

	// Create objects
	_window = factory.createWindow("windowName", this, 320, 200, 0, 0);
	_txtTitle = new Text(300, 17, 16, 6);
	_txtInit = new Text(300, 10, 16, 24);
	_lstDefenses = new TextList(300, 128, 16, 40);
	_btnOk = new TextButton(120, 18, 100, 170);
	_btnStart = new TextButton(148, 16, 8, 176);
	_btnAbort = new TextButton(148, 16, 164, 176);

	// Set palette
	setInterface("baseDefense");

	add(_window, "window", "baseDefense");
	add(_btnOk, "button", "baseDefense");
	add(_btnStart, "button", "baseDefense");
	add(_btnAbort, "button", "baseDefense");
	add(_txtTitle, "text", "baseDefense");
	add(_txtInit, "text", "baseDefense");
	add(_lstDefenses, "text", "baseDefense");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "baseDefense");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&BaseDefenseState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&BaseDefenseState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&BaseDefenseState::btnOkClick, Options::keyCancel);
	_btnOk->setVisible(false);

	_btnStart->setText(tr("STR_START_FIRING"));
	_btnStart->onMouseClick((ActionHandler)&BaseDefenseState::btnStartClick);

	_btnAbort->setText(tr("STR_SKIP_FIRING"));
	_btnAbort->onMouseClick((ActionHandler)&BaseDefenseState::btnOkClick);

	_txtTitle->setBig();
	_txtTitle->setText(tr("STR_BASE_UNDER_ATTACK").arg(_base->getName()));
	_txtInit->setVisible(false);

	_txtInit->setText(tr("STR_BASE_DEFENSES_INITIATED"));

	_lstDefenses->setColumns(3, 134, 70, 50);
	_lstDefenses->setFlooding(true);
	_gravShields = _base->getGravShields();
	_defenses = (int)_base->getDefenses().size();
	_timer = new Timer(250);
	_timer->onState(std::bind(&BaseDefenseState::nextStep, this));

	_explosionCount = 0;

	if (_ufo->getRules()->getMissilePower() != 0)
	{
		btnStartClick(0);
	}
}

/**
 *
 */
BaseDefenseState::~BaseDefenseState()
{
	delete _timer;
}

void BaseDefenseState::update()
{
	_timer->think(true, false);
}

void BaseDefenseState::nextStep()
{
	if (_thinkcycles == -1)
		return;

	++_thinkcycles;

	if (_thinkcycles == 1)
	{
		_txtInit->setVisible(true);
		return;
	}

	if (_thinkcycles > 1)
	{
		switch (_action)
		{
		case BDA_DESTROY:
			if (!_explosionCount)
			{
				_lstDefenses->addRow(2, tr("STR_UFO_DESTROYED").c_str()," "," ");
				++_row;
				if (_row > 14)
				{
					_lstDefenses->scrollDown(true);
				}
			}
			getGame()->getMod()->getSound("GEO.CAT", Mod::UFO_EXPLODE)->play();
			if (++_explosionCount == 3)
			{
				_action = BDA_END;
			}
			return;
		case BDA_END:
			_btnOk->setVisible(true);
			_thinkcycles = -1;
			return;
		default:
			break;
		}
		if (_attacks == _defenses && _passes == _gravShields)
		{
			_action = BDA_END;
			return;
		}
		else if (_attacks == _defenses && _passes < _gravShields)
		{
			_lstDefenses->addRow(3, tr("STR_GRAV_SHIELD_REPELS_UFO").c_str()," "," ");
			if (_row > 14)
			{
				_lstDefenses->scrollDown(true);
			}
			++_row;
			++_passes;
			_attacks = 0;
			return;
		}



		BaseFacility* def = _base->getDefenses().at(_attacks);
		const RuleItem* ammo = (def)->getRules()->getAmmoItem();
		int ammoNeeded = (def)->getRules()->getAmmoNeeded();
		bool hasOwnAmmo = def->getRules()->getAmmoMax() > 0;
		bool spendAmmo = false;

		switch (_action)
		{
		case  BDA_NONE:
			_lstDefenses->addRow(3, tr((def)->getRules()->getType()).c_str()," "," ");
			++_row;
			_action = BDA_FIRE;
			if (_row > 14)
			{
				_lstDefenses->scrollDown(true);
			}
			return;
		case BDA_FIRE:
			if (hasOwnAmmo && def->getAmmo() < ammoNeeded)
			{
				_lstDefenses->setCellText(_row, 1, tr("STR_NO_AMMO"));
			}
			else if (!hasOwnAmmo && ammo && _base->getStorageItems()->getItem(ammo) < ammoNeeded)
			{
				_lstDefenses->setCellText(_row, 1, tr("STR_NO_AMMO"));
			}
			else
			{
				_lstDefenses->setCellText(_row, 1, tr("STR_FIRING"));
				getGame()->getMod()->getSound("GEO.CAT", (def)->getRules()->getFireSound())->play();
			}
			_timer->setInterval(333);
			_action = BDA_RESOLVE;
			return;
		case BDA_RESOLVE:
			if (hasOwnAmmo && def->getAmmo() < ammoNeeded)
			{
				//_lstDefenses->setCellText(_row, 2, tr("STR_NO_AMMO"));
			}
			else if (!hasOwnAmmo && ammo && _base->getStorageItems()->getItem(ammo) < ammoNeeded)
			{
				//_lstDefenses->setCellText(_row, 2, tr("STR_NO_AMMO"));
			}
			else if (!RNG::percent((def)->getRules()->getHitRatio()))
			{
				spendAmmo = true;
				_lstDefenses->setCellText(_row, 2, tr("STR_MISSED"));
			}
			else
			{
				spendAmmo = true;
				_lstDefenses->setCellText(_row, 2, tr("STR_HIT"));
				getGame()->getMod()->getSound("GEO.CAT", (def)->getRules()->getHitSound())->play();
				int dmg = (def)->getRules()->getDefenseValue();
				dmg = dmg / 2 + RNG::generate(0, dmg);
				if (_ufo->getShield() > 0)
				{
					int shieldDamage = dmg;
					dmg = std::max(0, dmg - _ufo->getShield());
					_ufo->setShield(_ufo->getShield() - shieldDamage);
				}
				_ufo->setDamage(_ufo->getDamage() + dmg, getGame()->getMod());
			}
			if (spendAmmo && ammoNeeded > 0)
			{
				if (hasOwnAmmo)
				{
					def->setAmmo(def->getAmmo() - ammoNeeded);
					def->resetAmmoMissingReported();
				}
				else if (!hasOwnAmmo && ammo)
				{
					_base->getStorageItems()->removeItem(ammo, ammoNeeded);
				}
			}
			if (_ufo->getStatus() == Ufo::DESTROYED)
				_action = BDA_DESTROY;
			else
				_action = BDA_NONE;
			++_attacks;
			_timer->setInterval(250);
			return;
		default:
			break;
		}
	}
}

/**
* Starts base defense
* @param action Pointer to an action.
*/
void BaseDefenseState::btnStartClick(Action *)
{
	_btnStart->setVisible(false);
	_btnAbort->setVisible(false);
	_timer->start();
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void BaseDefenseState::btnOkClick(Action *)
{
	_timer->stop();
	getGame()->popState();
	if (_ufo->getStatus() != Ufo::DESTROYED)
	{
		_state->handleBaseDefense(_base, _ufo);
	}
	else
	{
		_base->cleanupDefenses(true);

		// instant retaliation mission only spawns one UFO and then ends
		if (_ufo->getMission()->getRules().getObjective() == OBJECTIVE_INSTANT_RETALIATION)
		{
			_ufo->getMission()->setInterrupted(true);
		}

		// aliens are not stupid and should stop trying eventually
		if (_ufo->getMission()->getRules().getObjective() == OBJECTIVE_RETALIATION && RNG::percent(getGame()->getMod()->getChanceToStopRetaliation()))
		{
			// unmark base...
			_base->setRetaliationTarget(false);

			AlienMission* am = _base->getRetaliationMission();
			if (!am)
			{
				// backwards-compatibility
				// apparently falling back to first region is wrong, but that's how it is in OXC. Not sure what would be right :P
				const Region* region = AreaSystem::locateValue<Region>(*_base);				
				const std::string regionName = region ? region->getRules()->getType()
													// fall back to first region. This dies if *no* regions, but thats probably okay
													  : getRegistry().front<Region>().get<Region>().getRules()->getType();
				am = getGame()->getSavedGame()->findAlienMission(regionName, OBJECTIVE_RETALIATION);
			}

			if (am && am->getRules().isMultiUfoRetaliation())
			{
				// Remember that more UFOs may be coming
				am->setMultiUfoRetaliationInProgress(true);
			}
			else
			{
				// Delete the mission and any live UFOs
				getGame()->getSavedGame()->deleteRetaliationMission(am, _base);
			}
		}
	}
}

}
