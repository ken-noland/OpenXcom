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
#include <cmath>
#include <sstream>
#include "BaseInfoState.h"
#include "BasescapeState.h"
#include "MiniBaseView.h"
#include "MonthlyCostsState.h"
#include "StoresState.h"
#include "TransfersState.h"
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Engine/Surface.h"
#include "../Entity/Game/BasescapeData.h"
#include "../Entity/Interface/Interface.h"
#include "../Interface/Bar.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextEdit.h"
#include "../Mod/Mod.h"
#include "../Savegame/Base.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Base Info screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param state Pointer to the Basescape state.
 */
BaseInfoState::BaseInfoState(BasescapeSystem& basescapeSystem)
	: State("BaseInfoState", true), _basescapeSystem(basescapeSystem)
{
	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();
	_unsubscribeId = _basescapeSystem.connectListener(std::bind(&BaseInfoState::init, this));

	// Create objects
	_bg = new Surface(320, 200, 0, 0);
	_mini = new MiniBaseView(_basescapeSystem, 128, 16, 182, 8);
	_btnOk = new TextButton(30, 14, 10, 180);
	_btnTransfers = new TextButton(80, 14, 46, 180);
	_btnStores = new TextButton(80, 14, 132, 180);
	_btnMonthlyCosts = new TextButton(92, 14, 218, 180);
	_edtBase = new TextEdit(this, 127, 16, 8, 8);

	_txtPersonnel = new Text(300, 9, 8, 30);
	_txtSoldiers = new Text(114, 9, 8, 41);
	_numSoldiers = new Text(40, 9, 126, 41);
	_barSoldiers = new Bar(150, 5, 166, 43);
	_txtEngineers = new Text(114, 9, 8, 51);
	_numEngineers = new Text(40, 9, 126, 51);
	_barEngineers = new Bar(150, 5, 166, 53);
	_txtScientists = new Text(114, 9, 8, 61);
	_numScientists = new Text(40, 9, 126, 61);
	_barScientists = new Bar(150, 5, 166, 63);

	_txtSpace = new Text(300, 9, 8, 72);
	_txtQuarters = new Text(114, 9, 8, 83);
	_numQuarters = new Text(40, 9, 126, 83);
	_barQuarters = new Bar(150, 5, 166, 85);
	_txtStores = new Text(114, 9, 8, 93);
	_numStores = new Text(40, 9, 126, 93);
	_barStores = new Bar(150, 5, 166, 95);
	_txtLaboratories = new Text(114, 9, 8, 103);
	_numLaboratories = new Text(40, 9, 126, 103);
	_barLaboratories = new Bar(150, 5, 166, 105);
	_txtWorkshops = new Text(114, 9, 8, 113);
	_numWorkshops = new Text(40, 9, 126, 113);
	_barWorkshops = new Bar(150, 5, 166, 115);
	if (Options::storageLimitsEnforced)
	{
		_txtContainment = new Text(114, 9, 8, 123);
		_numContainment = new Text(40, 9, 126, 123);
		_barContainment = new Bar(150, 5, 166, 125);
	}
	else
	{
		_txtContainment = nullptr;
		_numContainment = nullptr;
		_barContainment = nullptr;
	}
	_txtHangars = new Text(114, 9, 8, Options::storageLimitsEnforced ? 133 : 123);
	_numHangars = new Text(40, 9, 126, Options::storageLimitsEnforced ? 133 : 123);
	_barHangars = new Bar(150, 5, 166, Options::storageLimitsEnforced ? 135 : 125);

	_txtDefense = new Text(114, 9, 8, Options::storageLimitsEnforced ? 147 : 138);
	_numDefense = new Text(36, 9, 126, Options::storageLimitsEnforced ? 147 : 138);
	_barDefense = new Bar(150, 5, 166, Options::storageLimitsEnforced ? 149 : 140);
	_txtShortRange = new Text(114, 9, 8, Options::storageLimitsEnforced ? 157 : 153);
	_numShortRange = new Text(36, 9, 126, Options::storageLimitsEnforced ? 157 : 153);
	_barShortRange = new Bar(150, 5, 166, Options::storageLimitsEnforced ? 159 : 155);
	_txtLongRange = new Text(114, 9, 8, Options::storageLimitsEnforced ? 167 : 163);
	_numLongRange = new Text(36, 9, 126, Options::storageLimitsEnforced ? 167 : 163);
	_barLongRange = new Bar(150, 5, 166, Options::storageLimitsEnforced ? 169 : 165);

	// Set palette
	setInterface("baseInfo");

	add(_bg);
	add(_mini, "miniBase", "basescape");
	add(_btnOk, "button", "baseInfo");
	add(_btnTransfers, "button", "baseInfo");
	add(_btnStores, "button", "baseInfo");
	add(_btnMonthlyCosts, "button", "baseInfo");
	add(_edtBase, "text1", "baseInfo");

	add(_txtPersonnel, "text1", "baseInfo");
	add(_txtSoldiers, "text2", "baseInfo");
	add(_numSoldiers, "numbers", "baseInfo");
	add(_barSoldiers, "personnelBars", "baseInfo");
	add(_txtEngineers, "text2", "baseInfo");
	add(_numEngineers, "numbers", "baseInfo");
	add(_barEngineers, "personnelBars", "baseInfo");
	add(_txtScientists, "text2", "baseInfo");
	add(_numScientists, "numbers", "baseInfo");
	add(_barScientists, "personnelBars", "baseInfo");

	add(_txtSpace, "text1", "baseInfo");
	add(_txtQuarters, "text2", "baseInfo");
	add(_numQuarters, "numbers", "baseInfo");
	add(_barQuarters, "facilityBars", "baseInfo");
	add(_txtStores, "text2", "baseInfo");
	add(_numStores, "numbers", "baseInfo");
	add(_barStores, "facilityBars", "baseInfo");
	add(_txtLaboratories, "text2", "baseInfo");
	add(_numLaboratories, "numbers", "baseInfo");
	add(_barLaboratories, "facilityBars", "baseInfo");
	add(_txtWorkshops, "text2", "baseInfo");
	add(_numWorkshops, "numbers", "baseInfo");
	add(_barWorkshops, "facilityBars", "baseInfo");
	if (Options::storageLimitsEnforced)
	{
		add(_txtContainment, "text2", "baseInfo");
		add(_numContainment, "numbers", "baseInfo");
		add(_barContainment, "facilityBars", "baseInfo");
	}
	add(_txtHangars, "text2", "baseInfo");
	add(_numHangars, "numbers", "baseInfo");
	add(_barHangars, "facilityBars", "baseInfo");

	add(_txtDefense, "text2", "baseInfo");
	add(_numDefense, "numbers", "baseInfo");
	add(_barDefense, "defenceBar", "baseInfo");
	add(_txtShortRange, "text2", "baseInfo");
	add(_numShortRange, "numbers", "baseInfo");
	add(_barShortRange, "detectionBars", "baseInfo");
	add(_txtLongRange, "text2", "baseInfo");
	add(_numLongRange, "numbers", "baseInfo");
	add(_barLongRange, "detectionBars", "baseInfo");

	centerAllSurfaces();

	// Set up objects
	std::ostringstream ss;
	if (Options::storageLimitsEnforced)
	{
		ss << "ALT";
	}
	ss << "BACK07.SCR";
	getGame()->getMod()->getSurface(ss.str())->blitNShade(_bg, 0, 0);

	_mini->setTexture(getGame()->getMod()->getSurfaceSet("BASEBITS.PCK"));

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&BaseInfoState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&BaseInfoState::btnOkClick, Options::keyCancel);

	_btnTransfers->setText(tr("STR_TRANSFERS_UC"));
	_btnTransfers->onMouseClick((ActionHandler)&BaseInfoState::btnTransfersClick);

	_btnStores->setText(tr("STR_STORES_UC"));
	_btnStores->onMouseClick((ActionHandler)&BaseInfoState::btnStoresClick);

	_btnMonthlyCosts->setText(tr("STR_MONTHLY_COSTS"));
	_btnMonthlyCosts->onMouseClick((ActionHandler)&BaseInfoState::btnMonthlyCostsClick);

	_edtBase->setBig();
	_edtBase->onChange((ActionHandler)&BaseInfoState::edtBaseChange);

	_txtPersonnel->setText(tr("STR_PERSONNEL_AVAILABLE_PERSONNEL_TOTAL"));

	_txtSoldiers->setText(tr("STR_SOLDIERS"));

	_barSoldiers->setScale(1.0);

	_txtEngineers->setText(tr("STR_ENGINEERS"));

	_barEngineers->setScale(1.0);

	_txtScientists->setText(tr("STR_SCIENTISTS"));

	_barScientists->setScale(1.0);


	_txtSpace->setText(tr("STR_SPACE_USED_SPACE_AVAILABLE"));

	_txtQuarters->setText(tr("STR_LIVING_QUARTERS_PLURAL"));

	_barQuarters->setScale(0.5);

	_txtStores->setText(tr("STR_STORES"));

	_barStores->setScale(0.5);

	_txtLaboratories->setText(tr("STR_LABORATORIES"));

	_barLaboratories->setScale(0.5);

	_txtWorkshops->setText(tr("STR_WORK_SHOPS"));

	_barWorkshops->setScale(0.5);

	if (Options::storageLimitsEnforced)
	{
		_txtContainment->setText(tr("STR_ALIEN_CONTAINMENT"));

		_barContainment->setScale(0.5);
	}

	_txtHangars->setText(tr("STR_HANGARS"));

	_barHangars->setScale(18.0);


	_txtDefense->setText(tr("STR_DEFENSE_STRENGTH"));

	_barDefense->setScale(0.125);

	_txtShortRange->setText(tr("STR_SHORT_RANGE_DETECTION"));

	_barShortRange->setScale(25.0);

	_txtLongRange->setText(tr("STR_LONG_RANGE_DETECTION"));

	_barLongRange->setScale(25.0);
}

BaseInfoState::~BaseInfoState()
{
	_basescapeSystem.disconnectListener(_unsubscribeId);
}

static void setBarState(Text& barText, Bar& bar, int count, int max, double maxBarWidth)
{
	barText.setText(std::format("{}:{}", count, max));

	if (!Options::oxceBaseInfoScaleEnabled || count * bar.getScale() < maxBarWidth)
	{
		bar.setMax(max);
		bar.setValue(count);
	}
	else
	{
		bar.setMax(maxBarWidth);
		bar.setValue(count * maxBarWidth / max);
		bar.setScale(1.0);
	}
}

/**
 * The player can change the selected base.
 */
void BaseInfoState::init()
{
	State::init();
	Base& base = _basescapeSystem.getSelectedBase().get<Base>();
	_edtBase->setText(base.getName());

	setBarState(*_numSoldiers, *_barSoldiers, base.getAvailableSoldiers(), base.getTotalSoldiers(), MAX_BAR_WIDTH);
	setBarState(*_numEngineers, *_barEngineers, base.getAvailableEngineers(), base.getTotalEngineers(), MAX_BAR_WIDTH);
	setBarState(*_numScientists, *_barScientists, base.getAvailableScientists(), base.getTotalScientists(), MAX_BAR_WIDTH);
	setBarState(*_numQuarters, *_barQuarters, base.getUsedQuarters(), base.getAvailableQuarters(), MAX_BAR_WIDTH);
	setBarState(*_numStores, *_barStores, static_cast<int>(std::ceil(base.getUsedStores())), base.getAvailableStores(), MAX_BAR_WIDTH);
	setBarState(*_numLaboratories, *_barLaboratories, base.getUsedLaboratories(), base.getAvailableLaboratories(), MAX_BAR_WIDTH);
	setBarState(*_numWorkshops, *_barWorkshops, base.getUsedWorkshops(), base.getAvailableWorkshops(), MAX_BAR_WIDTH);

	if (Options::storageLimitsEnforced)
	{
		std::ostringstream ss72;
		ss72 << base.getUsedContainment(0) << ":" << base.getAvailableContainment(0);
		_numContainment->setText(ss72.str());

		_barContainment->setMax(base.getAvailableContainment(0));
		_barContainment->setValue(base.getUsedContainment(0));
	}

	std::ostringstream ss8;
	ss8 << base.getUsedHangars() << ":" << base.getAvailableHangars();
	_numHangars->setText(ss8.str());

	// bar dynamic scale

	if (base.getAvailableHangars() * _barHangars->getScale() < MAX_BAR_WIDTH)
	{
		_barHangars->setMax(base.getAvailableHangars());
		_barHangars->setValue(base.getUsedHangars());
	}
	else
	{
		_barHangars->setMax(MAX_BAR_WIDTH);
		_barHangars->setValue(base.getUsedHangars() * MAX_BAR_WIDTH / base.getAvailableHangars());
		_barHangars->setScale(1.0);
	}

	if (Options::baseDefenseProbability)
	{
		// display base defense percentage
		int defenseProbabilityPercentage = base.getDefenseProbabilityPercentage();
		_barDefense->setMax(base.getDefenseValue());
		_barDefense->setValue(base.getDefenseValue());
		if (Options::oxceBaseInfoDefenseScaleMultiplier != 100)
		{
			_barDefense->setScale(0.125 * Options::oxceBaseInfoDefenseScaleMultiplier / 100.0);
		}

		std::ostringstream ss9;
		ss9 << defenseProbabilityPercentage;
		ss9 << " %";
		_numDefense->setText(ss9.str());
		_numDefense->setAlign(ALIGN_RIGHT);

		_barDefense->setScale(1.0);
		_barDefense->setMax(100);
		_barDefense->setValue(defenseProbabilityPercentage);

	}
	else
	{
		// display base defense strength (vanilla)

		std::ostringstream ss9;
		ss9 << base.getDefenseValue();
		_numDefense->setText(ss9.str());

		_barDefense->setMax(base.getDefenseValue());
		_barDefense->setValue(base.getDefenseValue());

	}

	if (Options::baseDetectionProbability)
	{
		// display base detection probability percentage

		std::ostringstream ss10;
		int shortRangeDetectionProbabilityPercentage = base.getShortRangeDetectionProbabilityPercentage();
		ss10 << shortRangeDetectionProbabilityPercentage;
		ss10 << " %";
		_numShortRange->setText(ss10.str());
		_numShortRange->setAlign(ALIGN_RIGHT);

		_barShortRange->setScale(1.0);
		_barShortRange->setMax(100);
		_barShortRange->setValue(shortRangeDetectionProbabilityPercentage);

		std::ostringstream ss11;
		int longRangeDetectionProbabilityPercentage = base.getLongRangeDetectionProbabilityPercentage();
		ss11 << longRangeDetectionProbabilityPercentage;
		ss11 << " %";
		_numLongRange->setText(ss11.str());
		_numLongRange->setAlign(ALIGN_RIGHT);

		_barLongRange->setScale(1.0);
		_barLongRange->setMax(100);
		_barLongRange->setValue(longRangeDetectionProbabilityPercentage);

	}
	else
	{
		// display base detection count (vanilla)

		std::ostringstream ss10;
		int shortRangeDetection = base.getShortRangeDetection();
		ss10 << shortRangeDetection;
		_numShortRange->setText(ss10.str());

		_barShortRange->setMax(shortRangeDetection);
		_barShortRange->setValue(shortRangeDetection);

		std::ostringstream ss11;
		int longRangeDetection = base.getLongRangeDetection();
		ss11 << longRangeDetection;
		_numLongRange->setText(ss11.str());

		_barLongRange->setMax(longRangeDetection);
		_barLongRange->setValue(longRangeDetection);

	}
}

/**
 * Changes the base name.
 * @param action Pointer to an action.
 */
void BaseInfoState::edtBaseChange(Action *)
{
	_basescapeSystem.getSelectedBase().get<Base>().setName(_edtBase->getText());
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void BaseInfoState::btnOkClick(Action *)
{
	getGame()->popState();
}

/**
 * Goes to the Transfers window.
 * @param action Pointer to an action.
 */
void BaseInfoState::btnTransfersClick(Action *)
{
	getGame()->pushState(new TransfersState(&_basescapeSystem.getSelectedBase().get<Base>()));
}

/**
 * Goes to the Stores screen.
 * @param action Pointer to an action.
 */
void BaseInfoState::btnStoresClick(Action *)
{
	getGame()->pushState(new StoresState(&_basescapeSystem.getSelectedBase().get<Base>()));
}

/**
 * Goes to the Monthly Costs screen.
 * @param action Pointer to an action.
 */
void BaseInfoState::btnMonthlyCostsClick(Action *)
{
	getGame()->pushState(new MonthlyCostsState(&_basescapeSystem.getSelectedBase().get<Base>()));
}

}
