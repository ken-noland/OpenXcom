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
#include "SoldierDiaryMissionState.h"
#include "../Mod/Mod.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/SoldierDiary.h"
#include "../Savegame/MissionStatistics.h"
#include "../Savegame/BattleUnitStatistics.h"
#include "../Entity/Interface/Interface.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Soldier Diary Mission window.
 * @param soldier Pointer to the selected soldier.
 * @param rowEntry number to get mission info from.
 */
SoldierDiaryMissionState::SoldierDiaryMissionState(Soldier* soldier, int rowEntry) : State("SoldierDiaryMissionState", false), _soldier(soldier), _rowEntry(rowEntry)
{
	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();

	// Create object
	_window = factory.createWindow("soldierDiaryMission", this, 300, 128, 10, 36, WindowPopup::POPUP_HORIZONTAL);
	_btnOk = new TextButton(240, 16, 40, 140);
	_btnPrev = new TextButton(28, 14, 18, 44);
	_btnNext = new TextButton(28, 14, 274, 44);
	_txtTitle = new Text(262, 9, 29, 44);
	_txtUFO = new Text(262, 9, 29, 52);
	_txtScore = new Text(180, 9, 29, 68);
	_txtKills = new Text(120, 9, 169, 68);
	_txtLocation = new Text(180, 9, 29, 76);
	_txtRace = new Text(120, 9, 169, 76);
	_txtDaylight = new Text(120, 9, 169, 84);
	_txtDaysWounded = new Text(180, 9, 29, 84);
	_txtNoRecord = new Text(240, 9, 29, 100);
	_lstKills = new TextList(270, 32, 20, 100);

	// Set palette
	setInterface("soldierDiaryMission");

	add(_window, "window", "soldierDiaryMission");
	add(_btnOk, "button", "soldierDiaryMission");
	add(_btnPrev, "button", "soldierDiaryMission");
	add(_btnNext, "button", "soldierDiaryMission");
	add(_txtTitle, "text", "soldierDiaryMission");
	add(_txtUFO, "text", "soldierDiaryMission");
	add(_txtScore, "text", "soldierDiaryMission");
	add(_txtKills, "text", "soldierDiaryMission");
	add(_txtLocation, "text", "soldierDiaryMission");
	add(_txtRace, "text", "soldierDiaryMission");
	add(_txtDaylight, "text", "soldierDiaryMission");
	add(_txtDaysWounded, "text", "soldierDiaryMission");
	add(_txtNoRecord, "text", "soldierDiaryMission");
	add(_lstKills, "list", "soldierDiaryMission");

	centerAllSurfaces();

	// Set up object
	setWindowBackground(_window, "soldierDiaryMission");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&SoldierDiaryMissionState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&SoldierDiaryMissionState::btnOkClick, Options::keyCancel);

	_btnPrev->setText("<<");
	_btnPrev->onMouseClick((ActionHandler)&SoldierDiaryMissionState::btnPrevClick);
	_btnPrev->onKeyboardPress((ActionHandler)&SoldierDiaryMissionState::btnPrevClick, Options::keyBattleNextUnit);

	_btnNext->setText(">>");
	_btnNext->onMouseClick((ActionHandler)&SoldierDiaryMissionState::btnNextClick);
	_btnNext->onKeyboardPress((ActionHandler)&SoldierDiaryMissionState::btnNextClick, Options::keyBattlePrevUnit);

	_txtTitle->setAlign(TextHAlign::ALIGN_CENTER);

	_txtUFO->setAlign(TextHAlign::ALIGN_CENTER);

	_lstKills->setColumns(3, 60, 110, 100);
}

/**
 *
 */
SoldierDiaryMissionState::~SoldierDiaryMissionState()
{

}

/**
 *  Clears all the variables and reinitializes the stats for the mission.
 *
 */
void SoldierDiaryMissionState::init()
{
	State::init();
	if (_soldier->getDiary()->getMissionIdList().empty())
	{
		getGame()->popState();
		return;
	}
	std::vector<MissionStatistics*>& missionStatistics = getGame()->getSavedGame()->getMissionStatistics();
	unsigned int missionId = _soldier->getDiary()->getMissionIdList().at(_rowEntry);
	if (missionId > missionStatistics.size())
	{
		missionId = 0;
	}
	MissionStatistics *ms = missionStatistics.at(missionId);

	int daysWounded = 0;
	auto injuryIt = ms->injuryList.find(_soldier->getId());
	if (injuryIt != ms->injuryList.end())
	{
		daysWounded = (*injuryIt).second;
	}

	_lstKills->clearList();
	_txtTitle->setText(tr(ms->type));
	if (ms->isUfoMission())
	{
		_txtUFO->setText(tr(ms->ufo));
	}
	_txtUFO->setVisible(ms->isUfoMission());
	_txtScore->setText(tr("STR_SCORE_VALUE").arg(ms->score));
	_txtLocation->setText(tr("STR_LOCATION").arg(tr(ms->getLocationString())));
	_txtRace->setText(tr("STR_RACE_TYPE").arg(tr(ms->alienRace)));
	_txtRace->setVisible(ms->alienRace != "STR_UNKNOWN");
	_txtDaylight->setText(tr("STR_DAYLIGHT_TYPE").arg(tr(ms->getDaylightString(getGame()->getMod()))));
	_txtDaysWounded->setText(tr("STR_DAYS_WOUNDED").arg(daysWounded));
	_txtDaysWounded->setVisible(daysWounded != 0);

	int kills = 0;
	bool stunOrKill = false;

	for (const auto* battleUnitKills : _soldier->getDiary()->getKills())
	{
		if ((unsigned int)battleUnitKills->mission != missionId) continue;

		switch (battleUnitKills->status)
		{
		case STATUS_DEAD:
			kills++;
		//Fall-through
		case STATUS_UNCONSCIOUS:
		case STATUS_PANICKING:
		case STATUS_TURNING:
			stunOrKill = true;
		default:
			break;
		}

		_lstKills->addRow(3, tr(battleUnitKills->getKillStatusString()).c_str(),
							 battleUnitKills->getUnitName(getGame()->getLanguage()).c_str(),
							 tr(battleUnitKills->weapon).c_str());
	}

	_txtNoRecord->setAlign(TextHAlign::ALIGN_CENTER);
	_txtNoRecord->setText(tr("STR_NO_RECORD"));
	_txtNoRecord->setVisible(!stunOrKill);

	_txtKills->setText(tr("STR_KILLS").arg(kills));
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void SoldierDiaryMissionState::btnOkClick(Action *)
{
	getGame()->popState();
}

/**
 * Goes to the previous mission.
 * @param action Pointer to an action.
 */
void SoldierDiaryMissionState::btnPrevClick(Action *)
{
	if (_rowEntry == 0)
		_rowEntry = _soldier->getDiary()->getMissionTotal() - 1;
	else
		_rowEntry--;
	init();
}

/**
 * Goes to the next mission.
 * @param action Pointer to an action.
 */
void SoldierDiaryMissionState::btnNextClick(Action *)
{
	_rowEntry++;
	if (_rowEntry >= _soldier->getDiary()->getMissionTotal())
		_rowEntry = 0;
	init();
}

}
