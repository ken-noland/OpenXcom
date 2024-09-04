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
#include "BriefingState.h"
#include "BattlescapeState.h"
#include "BattlescapeGame.h"
#include "AliensCrashState.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/Text.h"
#include "../Interface/Window.h"
#include "InventoryState.h"
#include "NextTurnState.h"
#include "../Mod/Mod.h"
#include "../Savegame/Base.h"
#include "../Savegame/Craft.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Ufo.h"
#include "../Mod/AlienDeployment.h"
#include "../Mod/RuleUfo.h"
#include "../Engine/Options.h"
#include "../Engine/RNG.h"
#include "../Engine/Screen.h"
#include "../Menu/CutsceneState.h"
#include "../Savegame/AlienMission.h"
#include "../Mod/RuleAlienMission.h"
#include "../Entity/Interface/Interface.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Briefing screen.
 * @param game Pointer to the core game.
 * @param craft Pointer to the craft in the mission.
 * @param base Pointer to the base in the mission.
 * @param infoOnly Only show static info, when briefing is re-opened during the battle.
 * @param customBriefing Pointer to a custom briefing (used for Reinforcements notification).
 */
BriefingState::BriefingState(Craft* craft, Base* base, bool infoOnly, BriefingData* customBriefing) : State("BriefingState", true), _infoOnly(infoOnly), _disableCutsceneAndMusic(false)
{
	Options::baseXResolution = Options::baseXGeoscape;
	Options::baseYResolution = Options::baseYGeoscape;
	getGame()->getScreen()->resetDisplay(false);

	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();

	// Create objects
	_window = factory.createWindow("windowName", this, 320, 200, 0, 0);
	_btnOk = new TextButton(120, 18, 100, 164);
	_txtTitle = new Text(300, 32, 16, 24);
	_txtTarget = new Text(300, 17, 16, 40);
	_txtCraft = new Text(300, 17, 16, 56);
	_txtBriefing = new Text(274, 94, 16, 72);

	auto battleSave = getGame()->getSavedGame()->getSavedBattle();
	std::string mission = battleSave->getMissionType();
	AlienDeployment *deployment = getGame()->getMod()->getDeployment(mission);
	Ufo * ufo = 0;
	if (!deployment && craft)
	{
		ufo = dynamic_cast <Ufo*> (craft->getDestination());
		if (ufo) // landing site or crash site.
		{
			std::string ufoMissionName = ufo->getRules()->getType();
			if (!battleSave->getAlienCustomMission().empty())
			{
				// fake underwater UFO
				ufoMissionName = battleSave->getAlienCustomMission();
			}
			deployment = getGame()->getMod()->getDeployment(ufoMissionName);
		}
	}

	std::string title = mission;
	std::string desc = title + "_BRIEFING";

	WindowSystem& windowSystem = getGame()->getECS().getSystem<WindowSystem>();
	if (!deployment && !customBriefing) // none defined - should never happen, but better safe than sorry i guess.
	{
		setStandardPalette("PAL_GEOSCAPE", 0);
		_musicId = "GMDEFEND";
		windowSystem.setBackground(_window, getGame()->getMod()->getSurface("BACK16.SCR"));
	}
	else
	{
		BriefingData data = customBriefing ? *customBriefing : deployment->getBriefingData();
		setStandardPalette("PAL_GEOSCAPE", data.palette);
		windowSystem.setBackground(_window, getGame()->getMod()->getSurface(data.background));
		_txtCraft->setY(56 + data.textOffset);
		_txtBriefing->setY(72 + data.textOffset);
		_txtTarget->setVisible(data.showTarget);
		_txtCraft->setVisible(data.showCraft);
		_cutsceneId = data.cutscene;
		_musicId = data.music;
		if (!data.title.empty())
		{
			title = data.title;
		}
		if (!data.desc.empty())
		{
			desc = data.desc;
		}
	}
	_disableCutsceneAndMusic = _infoOnly && !customBriefing;

	add(_window, "window", "briefing");
	add(_btnOk, "button", "briefing");
	add(_txtTitle, "text", "briefing");
	add(_txtTarget, "text", "briefing");
	add(_txtCraft, "text", "briefing");
	add(_txtBriefing, "text", "briefing");

	centerAllSurfaces();

	// Set up objects
	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&BriefingState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&BriefingState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&BriefingState::btnOkClick, Options::keyCancel);

	_txtTitle->setBig();
	_txtTarget->setBig();
	_txtCraft->setBig();

	std::string s;
	if (craft)
	{
		if (craft->getDestination())
		{
			s = craft->getDestination()->getName(getGame()->getLanguage());
			battleSave->setMissionTarget(s);
		}

		s = tr("STR_CRAFT_").arg(craft->getName(getGame()->getLanguage()));
		battleSave->setMissionCraftOrBase(s);
	}
	else if (base)
	{
		s = tr("STR_BASE_UC_").arg(base->getName());
		battleSave->setMissionCraftOrBase(s);
	}

	// random operation names
	if (craft || base)
	{
		if (!getGame()->getMod()->getOperationNamesFirst().empty())
		{
			std::ostringstream ss;
			int pickFirst = RNG::seedless(0, (int)getGame()->getMod()->getOperationNamesFirst().size() - 1);
			ss << getGame()->getMod()->getOperationNamesFirst().at(pickFirst);
			if (!getGame()->getMod()->getOperationNamesLast().empty())
			{
				int pickLast = RNG::seedless(0, (int)getGame()->getMod()->getOperationNamesLast().size() - 1);
				ss << " " << getGame()->getMod()->getOperationNamesLast().at(pickLast);
			}
			s = ss.str();
			battleSave->setMissionTarget(s);
		}
	}

	if (!getGame()->getMod()->getOperationNamesFirst().empty())
		_txtTarget->setText(tr("STR_OPERATION_UC").arg(battleSave->getMissionTarget()));
	else
		_txtTarget->setText(battleSave->getMissionTarget());

	_txtCraft->setText(battleSave->getMissionCraftOrBase());

	_txtTitle->setText(tr(title));

	bool isPreview = battleSave->isPreview();
	if (isPreview)
	{
		if (battleSave->getCraftForPreview())
		{
			if (battleSave->getCraftForPreview()->getId() == RuleCraft::DUMMY_CRAFT_ID)
			{
				// we're using the same alienDeployment for the real craft preview and for the dummy craft preview,
				// but we want to have different briefing texts
				desc = desc + "_DUMMY";
			}
		}
		else
		{
			// base preview
			desc = desc + "_PREVIEW";
		}
	}
	_txtBriefing->setWordWrap(true);
	_txtBriefing->setText(tr(desc));

	if (_infoOnly) return;

	if (!isPreview && base && mission == "STR_BASE_DEFENSE")
	{
		auto* am = base->getRetaliationMission();

		// And make sure the base is unmarked (but only for vanilla retaliations, not for instant retaliations)
		if (am)
		{
			base->setRetaliationTarget(false);
		}

		if (am && am->getRules().isMultiUfoRetaliation())
		{
			// Remember that more UFOs may be coming
			am->setMultiUfoRetaliationInProgress(true);
		}
	}
}

/**
 *
 */
BriefingState::~BriefingState()
{

}

void BriefingState::init()
{
	State::init();
	if (_disableCutsceneAndMusic) return;

	if (!_cutsceneId.empty())
	{
		getGame()->pushState(new CutsceneState(_cutsceneId));

		// don't play the cutscene again when we return to this state
		_cutsceneId = "";
	}
	else
	{
		getGame()->getMod()->playMusic(_musicId);
	}
}

/**
 * Closes the window.
 * @param action Pointer to an action.
 */
void BriefingState::btnOkClick(Action *)
{
	getGame()->popState();
	Options::baseXResolution = Options::baseXBattlescape;
	Options::baseYResolution = Options::baseYBattlescape;
	getGame()->getScreen()->resetDisplay(false);
	if (_infoOnly) return;

	BattlescapeState *bs = new BattlescapeState;
	bs->getBattleGame()->spawnFromPrimedItems();
	auto tally = bs->getBattleGame()->tallyUnits();
	bool isPreview = getGame()->getSavedGame()->getSavedBattle()->isPreview();
	if (tally.liveAliens > 0 || isPreview)
	{
		getGame()->pushState(bs);
		getGame()->getSavedGame()->getSavedBattle()->setBattleState(bs);
		getGame()->pushState(new NextTurnState(getGame()->getSavedGame()->getSavedBattle(), bs));
		if (isPreview)
		{
			// skip InventoryState
			getGame()->getSavedGame()->getSavedBattle()->startFirstTurn();
			return;
		}
		getGame()->pushState(new InventoryState(false, bs, 0));
	}
	else
	{
		Options::baseXResolution = Options::baseXGeoscape;
		Options::baseYResolution = Options::baseYGeoscape;
		getGame()->getScreen()->resetDisplay(false);
		delete bs;
		getGame()->pushState(new AliensCrashState);
	}
}

}
