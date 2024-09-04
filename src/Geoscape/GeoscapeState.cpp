/*
 * Copyright 2010-2024 OpenXcom Developers.
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
#include "GeoscapeState.h"
#include <algorithm>
#include <climits>
#include <functional>
#include <iomanip>
#include <map>
#include <ranges>
#include <set>
#include <sstream>
#include "AlienBaseState.h"
#include "BaseDefenseState.h"
#include "BaseDestroyedState.h"
#include "ConfirmLandingState.h"
#include "CraftErrorState.h"
#include "CraftPatrolState.h"
#include "DogfightErrorState.h"
#include "DogfightExperienceState.h"
#include "DogfightState.h"
#include "ExtendedGeoscapeLinksState.h"
#include "FundingState.h"
#include "GeoscapeCraftState.h"
#include "GeoscapeEventState.h"
#include "Globe.h"
#include "GraphsState.h"
#include "HiddenAlienActivityState.h"
#include "InterceptState.h"
#include "ItemsArrivingState.h"
#include "LowFuelState.h"
#include "MissionDetectedState.h"
#include "MonthlyReportState.h"
#include "MultipleTargetsState.h"
#include "NewPossibleCraftState.h"
#include "NewPossibleFacilityState.h"
#include "NewPossibleManufactureState.h"
#include "NewPossiblePurchaseState.h"
#include "NewPossibleResearchState.h"
#include "ProductionCompleteState.h"
#include "ResearchCompleteState.h"
#include "ResearchRequiredState.h"
#include "SelectMusicTrackState.h"
#include "TrainingFinishedState.h"
#include "UfoDetectedState.h"
#include "UfoLostState.h"
#include "UfoTrackerState.h"
#include "../Basescape/BasescapeState.h"
#include "../Basescape/GlobalAlienContainmentState.h"
#include "../Basescape/GlobalManufactureState.h"
#include "../Basescape/GlobalResearchState.h"
#include "../Basescape/ManageAlienContainmentState.h"
#include "../Basescape/SellState.h"
#include "../Basescape/TechTreeViewerState.h"
#include "../Battlescape/BattlescapeGenerator.h"
#include "../Battlescape/BriefingState.h"
#include "../Engine/Action.h"
#include "../Engine/Collections.h"
#include "../Engine/Exception.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/RNG.h"
#include "../Engine/Screen.h"
#include "../Engine/Sound.h"
#include "../Engine/Surface.h"
#include "../Engine/Timer.h"
#include "../Engine/Unicode.h"
#include "../Entity/Engine/GeoSystem.h"
#include "../fallthrough.h"
#include "../fmath.h"
#include "../Interface/ComboBox.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Menu/CutsceneState.h"
#include "../Menu/ErrorMessageState.h"
#include "../Menu/ListSaveState.h"
#include "../Menu/LoadGameState.h"
#include "../Menu/PauseState.h"
#include "../Menu/SaveGameState.h"
#include "../Mod/AlienDeployment.h"
#include "../Mod/AlienRace.h"
#include "../Mod/Armor.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleAlienMission.h"
#include "../Mod/RuleArcScript.h"
#include "../Mod/RuleBaseFacility.h"
#include "../Mod/RuleCountry.h"
#include "../Mod/RuleCraft.h"
#include "../Mod/RuleEvent.h"
#include "../Mod/RuleEventScript.h"
#include "../Mod/RuleGlobe.h"
#include "../Mod/RuleInterface.h"
#include "../Mod/RuleManufacture.h"
#include "../Mod/RuleMissionScript.h"
#include "../Mod/RuleRegion.h"
#include "../Mod/RuleResearch.h"
#include "../Mod/RuleUfo.h"
#include "../Mod/RuleVideo.h"
#include "../Mod/UfoTrajectory.h"
#include "../Savegame/AlienBase.h"
#include "../Savegame/AlienMission.h"
#include "../Savegame/AlienStrategy.h"
#include "../Savegame/AreaSystem.h"
#include "../Savegame/Base.h"
#include "../Savegame/BaseFacility.h"
#include "../Savegame/BaseSystem.h"
#include "../Savegame/Country.h"
#include "../Savegame/CountrySystem.h"
#include "../Savegame/Craft.h"
#include "../Savegame/GameTime.h"
#include "../Savegame/GeoscapeEvent.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/MissionSite.h"
#include "../Savegame/Production.h"
#include "../Savegame/Region.h"
#include "../Savegame/ResearchProject.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/SoldierDiary.h"
#include "../Savegame/Transfer.h"
#include "../Savegame/Ufo.h"
#include "../Savegame/Waypoint.h"
#include "../Ufopaedia/Ufopaedia.h"

#include "../Entity/Engine/Surface.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Geoscape screen.
 * @param game Pointer to the core game.
 */
GeoscapeState::GeoscapeState()
	: State("GeoscapeState", true), _pause(false), _zoomInEffectDone(false), _zoomOutEffectDone(false), _minimizedDogfights(0), _slowdownCounter(0)
{
	int screenWidth = Options::baseXGeoscape;
	int screenHeight = Options::baseYGeoscape;

	// Create objects
	Surface *hd = getGame()->getMod()->getSurface("ALTGEOBORD.SCR");
	_bg = new Surface(hd->getWidth(), hd->getHeight(), 0, 0);
	_sideLine = new Surface(64, screenHeight, screenWidth - 64, 0);
	_sidebar = new Surface(64, 200, screenWidth - 64, screenHeight / 2 - 100);

	_globe = new Globe(getGame(), (screenWidth-64)/2, screenHeight/2, screenWidth-64, screenHeight, 0, 0);
	_bg->setX((_globe->getWidth() - _bg->getWidth()) / 2);
	_bg->setY((_globe->getHeight() - _bg->getHeight()) / 2);

	_btnIntercept = new TextButton(63, 11, screenWidth-63, screenHeight/2-100);
	_btnBases = new TextButton(63, 11, screenWidth-63, screenHeight/2-88);
	_btnGraphs = new TextButton(63, 11, screenWidth-63, screenHeight/2-76);
	_btnUfopaedia = new TextButton(63, 11, screenWidth-63, screenHeight/2-64);
	_btnOptions = new TextButton(63, 11, screenWidth-63, screenHeight/2-52);
	_btnFunding = new TextButton(63, 11, screenWidth-63, screenHeight/2-40);

	_btn5Secs = new TextButton(31, 13, screenWidth-63, screenHeight/2+12);
	_btn1Min = new TextButton(31, 13, screenWidth-31, screenHeight/2+12);
	_btn5Mins = new TextButton(31, 13, screenWidth-63, screenHeight/2+26);
	_btn30Mins = new TextButton(31, 13, screenWidth-31, screenHeight/2+26);
	_btn1Hour = new TextButton(31, 13, screenWidth-63, screenHeight/2+40);
	_btn1Day = new TextButton(31, 13, screenWidth-31, screenHeight/2+40);

	_btnRotateLeft = new InteractiveSurface(12, 10, screenWidth-61, screenHeight/2+76);
	_btnRotateRight = new InteractiveSurface(12, 10, screenWidth-37, screenHeight/2+76);
	_btnRotateUp = new InteractiveSurface(13, 12, screenWidth-49, screenHeight/2+62);
	_btnRotateDown = new InteractiveSurface(13, 12, screenWidth-49, screenHeight/2+87);
	_btnZoomIn = new InteractiveSurface(23, 23, screenWidth-25, screenHeight/2+56);
	_btnZoomOut = new InteractiveSurface(13, 17, screenWidth-20, screenHeight/2+82);

	int height = (screenHeight - Screen::ORIGINAL_HEIGHT) / 2 + 10;
	_sideTop = new TextButton(63, height, screenWidth-63, _sidebar->getY() - height - 1);
	_sideBottom = new TextButton(63, height, screenWidth-63, _sidebar->getY() + _sidebar->getHeight() + 1);

	_txtHour = new Text(20, 16, screenWidth-61, screenHeight/2-26);
	_txtHourSep = new Text(4, 16, screenWidth-41, screenHeight/2-26);
	_txtMin = new Text(20, 16, screenWidth-37, screenHeight/2-26);
	_txtMinSep = new Text(4, 16, screenWidth-17, screenHeight/2-26);
	_txtSec = new Text(11, 8, screenWidth-13, screenHeight/2-20);
	_txtWeekday = new Text(59, 8, screenWidth-61, screenHeight/2-13);
	_txtDay = new Text(29, 8, screenWidth-61, screenHeight/2-6);
	_txtMonth = new Text(29, 8, screenWidth-32, screenHeight/2-6);
	_txtYear = new Text(59, 8, screenWidth-61, screenHeight/2+1);
	_txtFunds = new Text(59, 8, screenWidth-61, screenHeight/2-27);

	int slackingIndicatorOffset = getGame()->getMod()->getInterface("geoscape")->getElement("slackingIndicator")->custom;
	_txtSlacking = new Text(59, 17, screenWidth - 61, screenHeight / 2 - 100 + slackingIndicatorOffset);

	_timeSpeed = _btn5Secs;
	_gameTimer = new Timer(Options::geoClockSpeed);

	_zoomInEffectTimer = new Timer(Options::dogfightSpeed);
	_zoomOutEffectTimer = new Timer(Options::dogfightSpeed);
	_dogfightStartTimer = new Timer(Options::dogfightSpeed);
	_dogfightTimer = new Timer(Options::dogfightSpeed);

	_txtDebug = new Text(254, 32, 0, 0);
	_cbxRegion = new ComboBox(this, 150, 16, 0, 36);
	_cbxZone = new ComboBox(this, 48, 16, 154, 36);
	_cbxArea = new ComboBox(this, 48, 16, 206, 36);
	_cbxCountry = new ComboBox(this, 150, 16, 0, 36);

	// Set palette
	setInterface("geoscape");

	add(_bg);
	add(_sideLine);
	add(_sidebar);
	add(_globe);

	add(_btnIntercept, "button", "geoscape");
	add(_btnBases, "button", "geoscape");
	add(_btnGraphs, "button", "geoscape");
	add(_btnUfopaedia, "button", "geoscape");
	add(_btnOptions, "button", "geoscape");
	add(_btnFunding, "button", "geoscape");

	add(_btn5Secs, "button", "geoscape");
	add(_btn1Min, "button", "geoscape");
	add(_btn5Mins, "button", "geoscape");
	add(_btn30Mins, "button", "geoscape");
	add(_btn1Hour, "button", "geoscape");
	add(_btn1Day, "button", "geoscape");

	add(_btnRotateLeft);
	add(_btnRotateRight);
	add(_btnRotateUp);
	add(_btnRotateDown);
	add(_btnZoomIn);
	add(_btnZoomOut);

	add(_sideTop, "button", "geoscape");
	add(_sideBottom, "button", "geoscape");

	add(_txtFunds, "text", "geoscape");
	add(_txtHour, "text", "geoscape");
	add(_txtHourSep, "text", "geoscape");
	add(_txtMin, "text", "geoscape");
	add(_txtMinSep, "text", "geoscape");
	add(_txtSec, "text", "geoscape");
	add(_txtWeekday, "text", "geoscape");
	add(_txtDay, "text", "geoscape");
	add(_txtMonth, "text", "geoscape");
	add(_txtYear, "text", "geoscape");
	add(_txtSlacking, "slackingIndicator", "geoscape");

	add(_txtDebug, "text", "geoscape");
	add(_cbxRegion, "button", "geoscape");
	add(_cbxZone, "button", "geoscape");
	add(_cbxArea, "button", "geoscape");
	add(_cbxCountry, "button", "geoscape");

	// Set up objects
	Surface *geobord = getGame()->getMod()->getSurface("GEOBORD.SCR");
	geobord->setX(_sidebar->getX() - geobord->getWidth() + _sidebar->getWidth());
	geobord->setY(_sidebar->getY());
	_sidebar->copy(geobord);
	getGame()->getMod()->getSurface("ALTGEOBORD.SCR")->blitNShade(_bg, 0, 0);

	_sideLine->drawRect(0, 0, _sideLine->getWidth(), _sideLine->getHeight(), 15);

	_btnIntercept->initText(getGame()->getMod()->getFont("FONT_GEO_BIG"), getGame()->getMod()->getFont("FONT_GEO_SMALL"), getGame()->getLanguage());
	_btnIntercept->setText(tr("STR_INTERCEPT"));
	_btnIntercept->onMouseClick((ActionHandler)&GeoscapeState::btnInterceptClick);
	_btnIntercept->onKeyboardPress((ActionHandler)&GeoscapeState::btnInterceptClick, Options::keyGeoIntercept);
	_btnIntercept->onKeyboardPress((ActionHandler)&GeoscapeState::btnUfoTrackerClick, Options::keyGeoUfoTracker);
	_btnIntercept->onKeyboardPress((ActionHandler)&GeoscapeState::btnTechTreeViewerClick, Options::keyGeoTechTreeViewer);
	_btnIntercept->onKeyboardPress((ActionHandler)&GeoscapeState::btnSelectMusicTrackClick, Options::keySelectMusicTrack);
	_btnIntercept->onKeyboardPress((ActionHandler)&GeoscapeState::btnGlobalProductionClick, Options::keyGeoGlobalProduction);
	_btnIntercept->onKeyboardPress((ActionHandler)&GeoscapeState::btnGlobalResearchClick, Options::keyGeoGlobalResearch);
	_btnIntercept->onKeyboardPress((ActionHandler)&GeoscapeState::btnGlobalAlienContainmentClick, Options::keyGeoGlobalAlienContainment);
	_btnIntercept->onKeyboardPress((ActionHandler)&GeoscapeState::btnDogfightExperienceClick, Options::keyGeoDailyPilotExperience);
	_btnIntercept->setGeoscapeButton(true);

	_btnBases->initText(getGame()->getMod()->getFont("FONT_GEO_BIG"), getGame()->getMod()->getFont("FONT_GEO_SMALL"), getGame()->getLanguage());
	_btnBases->setText(tr("STR_BASES"));
	_btnBases->onMouseClick((ActionHandler)&GeoscapeState::btnBasesClick);
	_btnBases->onKeyboardPress((ActionHandler)&GeoscapeState::btnBasesClick, Options::keyGeoBases);
	_btnBases->setGeoscapeButton(true);

	_btnGraphs->initText(getGame()->getMod()->getFont("FONT_GEO_BIG"), getGame()->getMod()->getFont("FONT_GEO_SMALL"), getGame()->getLanguage());
	_btnGraphs->setText(tr("STR_GRAPHS"));
	_btnGraphs->onMouseClick((ActionHandler)&GeoscapeState::btnGraphsClick);
	_btnGraphs->onKeyboardPress((ActionHandler)&GeoscapeState::btnGraphsClick, Options::keyGeoGraphs);
	_btnGraphs->setGeoscapeButton(true);

	_btnUfopaedia->initText(getGame()->getMod()->getFont("FONT_GEO_BIG"), getGame()->getMod()->getFont("FONT_GEO_SMALL"), getGame()->getLanguage());
	_btnUfopaedia->setText(tr("STR_UFOPAEDIA_UC"));
	_btnUfopaedia->onMouseClick((ActionHandler)&GeoscapeState::btnUfopaediaClick);
	_btnUfopaedia->onKeyboardPress((ActionHandler)&GeoscapeState::btnUfopaediaClick, Options::keyGeoUfopedia);
	_btnUfopaedia->setGeoscapeButton(true);

	_btnOptions->initText(getGame()->getMod()->getFont("FONT_GEO_BIG"), getGame()->getMod()->getFont("FONT_GEO_SMALL"), getGame()->getLanguage());
	_btnOptions->setText(tr("STR_OPTIONS_UC"));
	_btnOptions->onMouseClick((ActionHandler)&GeoscapeState::btnOptionsClick);
	_btnOptions->onKeyboardPress((ActionHandler)&GeoscapeState::btnOptionsClick, Options::keyGeoOptions);
	_btnOptions->setGeoscapeButton(true);

	_btnFunding->initText(getGame()->getMod()->getFont("FONT_GEO_BIG"), getGame()->getMod()->getFont("FONT_GEO_SMALL"), getGame()->getLanguage());
	_btnFunding->setText(Options::oxceLinks ? tr("STR_EXTENDED_UC") : tr("STR_FUNDING_UC"));
	_btnFunding->onMouseClick((ActionHandler)&GeoscapeState::btnFundingClick);
	_btnFunding->onKeyboardPress((ActionHandler)&GeoscapeState::btnFundingClick, Options::keyGeoFunding);
	_btnFunding->setGeoscapeButton(true);

	_btn5Secs->initText(getGame()->getMod()->getFont("FONT_GEO_BIG"), getGame()->getMod()->getFont("FONT_GEO_SMALL"), getGame()->getLanguage());
	_btn5Secs->setBig();
	_btn5Secs->setText(tr("STR_5_SECONDS"));
	_btn5Secs->setGroup(&_timeSpeed);
	_btn5Secs->onKeyboardPress((ActionHandler)&GeoscapeState::btnTimerClick, Options::keyGeoSpeed1);
	_btn5Secs->setGeoscapeButton(true);

	_btn1Min->initText(getGame()->getMod()->getFont("FONT_GEO_BIG"), getGame()->getMod()->getFont("FONT_GEO_SMALL"), getGame()->getLanguage());
	_btn1Min->setBig();
	_btn1Min->setText(tr("STR_1_MINUTE"));
	_btn1Min->setGroup(&_timeSpeed);
	_btn1Min->onKeyboardPress((ActionHandler)&GeoscapeState::btnTimerClick, Options::keyGeoSpeed2);
	_btn1Min->setGeoscapeButton(true);

	_btn5Mins->initText(getGame()->getMod()->getFont("FONT_GEO_BIG"), getGame()->getMod()->getFont("FONT_GEO_SMALL"), getGame()->getLanguage());
	_btn5Mins->setBig();
	_btn5Mins->setText(tr("STR_5_MINUTES"));
	_btn5Mins->setGroup(&_timeSpeed);
	_btn5Mins->onKeyboardPress((ActionHandler)&GeoscapeState::btnTimerClick, Options::keyGeoSpeed3);
	_btn5Mins->setGeoscapeButton(true);

	_btn30Mins->initText(getGame()->getMod()->getFont("FONT_GEO_BIG"), getGame()->getMod()->getFont("FONT_GEO_SMALL"), getGame()->getLanguage());
	_btn30Mins->setBig();
	_btn30Mins->setText(tr("STR_30_MINUTES"));
	_btn30Mins->setGroup(&_timeSpeed);
	_btn30Mins->onKeyboardPress((ActionHandler)&GeoscapeState::btnTimerClick, Options::keyGeoSpeed4);
	_btn30Mins->setGeoscapeButton(true);

	_btn1Hour->initText(getGame()->getMod()->getFont("FONT_GEO_BIG"), getGame()->getMod()->getFont("FONT_GEO_SMALL"), getGame()->getLanguage());
	_btn1Hour->setBig();
	_btn1Hour->setText(tr("STR_1_HOUR"));
	_btn1Hour->setGroup(&_timeSpeed);
	_btn1Hour->onKeyboardPress((ActionHandler)&GeoscapeState::btnTimerClick, Options::keyGeoSpeed5);
	_btn1Hour->setGeoscapeButton(true);

	_btn1Day->initText(getGame()->getMod()->getFont("FONT_GEO_BIG"), getGame()->getMod()->getFont("FONT_GEO_SMALL"), getGame()->getLanguage());
	_btn1Day->setBig();
	_btn1Day->setText(tr("STR_1_DAY"));
	_btn1Day->setGroup(&_timeSpeed);
	_btn1Day->onKeyboardPress((ActionHandler)&GeoscapeState::btnTimerClick, Options::keyGeoSpeed6);
	_btn1Day->setGeoscapeButton(true);

	_sideBottom->setGeoscapeButton(true);
	_sideTop->setGeoscapeButton(true);

	_btnRotateLeft->onMousePress((ActionHandler)&GeoscapeState::btnRotateLeftPress);
	_btnRotateLeft->onMouseRelease((ActionHandler)&GeoscapeState::btnRotateLeftRelease);
	_btnRotateLeft->onKeyboardPress((ActionHandler)&GeoscapeState::btnRotateLeftPress, Options::keyGeoLeft);
	_btnRotateLeft->onKeyboardRelease((ActionHandler)&GeoscapeState::btnRotateLeftRelease, Options::keyGeoLeft);

	_btnRotateRight->onMousePress((ActionHandler)&GeoscapeState::btnRotateRightPress);
	_btnRotateRight->onMouseRelease((ActionHandler)&GeoscapeState::btnRotateRightRelease);
	_btnRotateRight->onKeyboardPress((ActionHandler)&GeoscapeState::btnRotateRightPress, Options::keyGeoRight);
	_btnRotateRight->onKeyboardRelease((ActionHandler)&GeoscapeState::btnRotateRightRelease, Options::keyGeoRight);

	_btnRotateUp->onMousePress((ActionHandler)&GeoscapeState::btnRotateUpPress);
	_btnRotateUp->onMouseRelease((ActionHandler)&GeoscapeState::btnRotateUpRelease);
	_btnRotateUp->onKeyboardPress((ActionHandler)&GeoscapeState::btnRotateUpPress, Options::keyGeoUp);
	_btnRotateUp->onKeyboardRelease((ActionHandler)&GeoscapeState::btnRotateUpRelease, Options::keyGeoUp);

	_btnRotateDown->onMousePress((ActionHandler)&GeoscapeState::btnRotateDownPress);
	_btnRotateDown->onMouseRelease((ActionHandler)&GeoscapeState::btnRotateDownRelease);
	_btnRotateDown->onKeyboardPress((ActionHandler)&GeoscapeState::btnRotateDownPress, Options::keyGeoDown);
	_btnRotateDown->onKeyboardRelease((ActionHandler)&GeoscapeState::btnRotateDownRelease, Options::keyGeoDown);

	_btnZoomIn->onMouseClick((ActionHandler)&GeoscapeState::btnZoomInLeftClick, SDL_BUTTON_LEFT);
	_btnZoomIn->onMouseClick((ActionHandler)&GeoscapeState::btnZoomInRightClick, SDL_BUTTON_RIGHT);
	_btnZoomIn->onKeyboardPress((ActionHandler)&GeoscapeState::btnZoomInLeftClick, Options::keyGeoZoomIn);

	_btnZoomOut->onMouseClick((ActionHandler)&GeoscapeState::btnZoomOutLeftClick, SDL_BUTTON_LEFT);
	_btnZoomOut->onMouseClick((ActionHandler)&GeoscapeState::btnZoomOutRightClick, SDL_BUTTON_RIGHT);
	_btnZoomOut->onKeyboardPress((ActionHandler)&GeoscapeState::btnZoomOutLeftClick, Options::keyGeoZoomOut);

	_txtFunds->setAlign(TextHAlign::ALIGN_CENTER);
	_txtFunds->setVisible(Options::showFundsOnGeoscape);

	_txtHour->setBig();
	_txtHour->setAlign(TextHAlign::ALIGN_RIGHT);

	_txtHourSep->setBig();
	_txtHourSep->setText(":");

	_txtMin->setBig();

	_txtMinSep->setBig();
	_txtMinSep->setText(":");

	_txtWeekday->setAlign(TextHAlign::ALIGN_CENTER);

	_txtDay->setAlign(TextHAlign::ALIGN_CENTER);

	_txtMonth->setAlign(TextHAlign::ALIGN_CENTER);

	_txtYear->setAlign(TextHAlign::ALIGN_CENTER);

	_txtSlacking->setAlign(TextHAlign::ALIGN_RIGHT);

	if (Options::showFundsOnGeoscape)
	{
		_txtHour->setY(_txtHour->getY()+6);
		_txtHour->setSmall();
		_txtHourSep->setY(_txtHourSep->getY()+6);
		_txtHourSep->setSmall();
		_txtMin->setY(_txtMin->getY()+6);
		_txtMin->setSmall();
		_txtMinSep->setX(_txtMinSep->getX()-10);
		_txtMinSep->setY(_txtMinSep->getY()+6);
		_txtMinSep->setSmall();
		_txtSec->setX(_txtSec->getX()-10);
	}

	_gameTimer->onState(std::bind(&GeoscapeState::timeAdvance, this));
	_gameTimer->start();

	_zoomInEffectTimer->onState(std::bind(&GeoscapeState::zoomInEffect, this));
	_zoomOutEffectTimer->onState(std::bind(&GeoscapeState::zoomOutEffect, this));
	_dogfightStartTimer->onState(std::bind(&GeoscapeState::startDogfight, this));
	_dogfightTimer->onState(std::bind(&GeoscapeState::handleDogfights, this));

	// debug helpers
	{
		std::vector<std::string> regionList;
		regionList.push_back("All regions");
		_regionDebugComboBoxIndexToEntity.emplace(0, entt::null);
		size_t index = 1;
		for (auto&& [id, region] : getRegistry().raw().view<Region>().each())
		{
			_regionDebugComboBoxIndexToEntity.emplace(index++, id);
			regionList.push_back(region.getRules()->getType());
		}
		_cbxRegion->setOptions(regionList, false);
		_cbxRegion->setVisible(false);
		_cbxRegion->onChange((ActionHandler)&GeoscapeState::cbxRegionChange);

		std::vector<std::string> zoneList;
		zoneList.push_back("All zones");
		for (int z = 0; z < 20; ++z)
		{
			zoneList.push_back(std::to_string(z));
		}
		_cbxZone->setOptions(zoneList, false);
		_cbxZone->setVisible(false);
		_cbxZone->onChange((ActionHandler)&GeoscapeState::cbxZoneChange);

		std::vector<std::string> areaList;
		areaList.push_back("All areas");
		for (int z = 0; z < 100; ++z)
		{
			areaList.push_back(std::to_string(z));
		}
		_cbxArea->setOptions(areaList, false);
		_cbxArea->setVisible(false);
		_cbxArea->onChange((ActionHandler)&GeoscapeState::cbxAreaChange);

		std::vector<std::string> countryList;
		countryList.push_back("All countries");
		_countryDebugComboBoxIndexToEntity.emplace(0, entt::null);

		index = 1;
		for (const auto&& [id, country] : getRegistry().raw().view<const Country>().each())
		{
			_countryDebugComboBoxIndexToEntity.emplace(index++, id);
			countryList.push_back(tr(country.getRules()->getType()));
		}
		_cbxCountry->setOptions(countryList, false);
		_cbxCountry->setVisible(false);
		_cbxCountry->onChange((ActionHandler)&GeoscapeState::cbxCountryChange);
	}

	getRegistry().getService<GeoSystem>().updateAllRegionsAndCountries();

	timeDisplay();
}

/**
 * Deletes timers.
 */
GeoscapeState::~GeoscapeState()
{
	delete _gameTimer;
	delete _zoomInEffectTimer;
	delete _zoomOutEffectTimer;
	delete _dogfightStartTimer;
	delete _dogfightTimer;

	for (auto* dfs : _dogfights)
	{
		delete dfs;
	}
	_dogfights.clear();
	for (auto* dfs : _dogfightsToBeStarted)
	{
		delete dfs;
	}
	_dogfightsToBeStarted.clear();
}

/**
 * Handle blitting of Geoscape and Dogfights.
 */
void GeoscapeState::blit()
{
	State::blit();
	for (auto* dfs : _dogfights)
	{
		dfs->blit();
	}
}

/**
 * Handle key shortcuts.
 * @param action Pointer to an action.
 */
void GeoscapeState::handle(Action *action)
{
	if (_dogfights.size() == _minimizedDogfights)
	{
		State::handle(action);
	}

	if (action->getDetails()->type == SDL_KEYDOWN)
	{
		// "ctrl-d" - enable debug mode
		if (Options::debug && action->getDetails()->key.keysym.sym == SDLK_d && getGame()->isCtrlPressed())
		{
			btnDebugClick(nullptr);
		}
		if (Options::debug && getGame()->getSavedGame()->getDebugMode() && getGame()->isCtrlPressed())
		{
			// "ctrl-1"
			if (action->getDetails()->key.keysym.sym == SDLK_1)
			{
				_txtDebug->setText("I'M A BILLIONAIRE! ALMOST...");
				getGame()->getSavedGame()->setFunds(999999999);
			}
			// "ctrl-2"
			if (action->getDetails()->key.keysym.sym == SDLK_2)
			{
				_txtDebug->setText("ALL FACILITY CONSTRUCTION COMPLETED");
				for (Base& xcomBase : getRegistry().list<Base>())
				{
					for (BaseFacility* facility : xcomBase.getFacilities())
					{
						facility->setBuildTime(0);
						facility->setIfHadPreviousFacility(false);
					}
				}
			}
			// "ctrl-3"
			if (action->getDetails()->key.keysym.sym == SDLK_3)
			{
				_txtDebug->setText("+50 SCIENTISTS/ENGINEERS");
				for (Base& xcomBase : getRegistry().list<Base>())
				{
					xcomBase.setScientists(xcomBase.getScientists() + 50);
					xcomBase.setEngineers(xcomBase.getEngineers() + 50);
				}
			}
			// "ctrl-4"
			if (action->getDetails()->key.keysym.sym == SDLK_4)
			{
				_txtDebug->setText("+2 ALL ITEMS");
				for (Base& xcomBase : getRegistry().list<Base>())
				{
					for (const std::string& itemType : getGame()->getMod()->getItemsList())
					{
						RuleItem* item = getGame()->getMod()->getItem(itemType);
						if (item && item->isRecoverable() && !item->isAlien() && item->getSellCost() > 0)
						{
							xcomBase.getStorageItems()->addItem(item, 2);
						}
					}
				}
			}
			// "ctrl-5"
			if (action->getDetails()->key.keysym.sym == SDLK_5)
			{
				_txtDebug->setText("+2 ALL LIVE ALIENS");
				for (Base& xcomBase : getRegistry().list<Base>())
				{
					for (auto& itemType : getGame()->getMod()->getItemsList())
					{
						auto* item = getGame()->getMod()->getItem(itemType);
						if (item && item->isRecoverable() && item->isAlien() && item->getSellCost() > 0)
						{
							xcomBase.getStorageItems()->addItem(item, 2);
						}
					}
				}
			}
			// "ctrl-6"
			if (action->getDetails()->key.keysym.sym == SDLK_6)
			{
				_txtDebug->setText("XCOM/ALIEN ACTIVITY FOR THIS MONTH RESET");
				size_t invertedEntry = getGame()->getSavedGame()->getFundsList().size() - 1;
				for (Region& region : getRegistry().list<Region>())
				{
					region.getActivityXcom().at(invertedEntry) = 0;
					region.getActivityAlien().at(invertedEntry) = 0;
				}
				for (Country& country : getRegistry().list<Country>())
				{
					country.getActivityXcom().at(invertedEntry) = 0;
					country.getActivityAlien().at(invertedEntry) = 0;
				}
			}
			// "ctrl-7"
			if (action->getDetails()->key.keysym.sym == SDLK_7)
			{
				_txtDebug->setText("BIG BROTHER SEES ALL");
				for (Ufo& ufo : getRegistry().list<Ufo>())
				{
					ufo.setDetected(true);
					ufo.setHyperDetected(true);
				}
				for (AlienBase* ab : getGame()->getSavedGame()->getAlienBases())
				{
					ab->setDiscovered(true);
				}
			}
			// "ctrl-a"
			if (action->getDetails()->key.keysym.sym == SDLK_a)
			{
				_txtDebug->setText("SOLDIER DIARIES DELETED");
				for (Base& xcomBase : getRegistry().list<Base>())
				{
					for (Soldier* soldier : xcomBase.getSoldiers())
					{
						soldier->resetDiary();
					}
				}
			}
			// "ctrl-c"
			if (action->getDetails()->key.keysym.sym == SDLK_c)
			{
				_txtDebug->setText("SOLDIER COMMENDATIONS DELETED");
				for (Base& xcomBase : getRegistry().list<Base>())
				{
					for (Soldier* soldier : xcomBase.getSoldiers())
					{
						for (SoldierCommendations* commendation : soldier->getDiary()->getSoldierCommendations())
						{
							delete commendation;
						}
						soldier->getDiary()->getSoldierCommendations().clear();
					}
				}
			}
		}
		// quick save and quick load
		if (!getGame()->getSavedGame()->isIronman())
		{
			if (action->getDetails()->key.keysym.sym == Options::keyQuickSave)
			{
				popup(new SaveGameState(OPT_GEOSCAPE, SAVE_QUICK, _palette));
			}
			else if (action->getDetails()->key.keysym.sym == Options::keyQuickLoad)
			{
				popup(new LoadGameState(OPT_GEOSCAPE, SAVE_QUICK, _palette));
			}
		}
	}
	if (!_dogfights.empty())
	{
		for (auto* dfs : _dogfights)
		{
			dfs->handle(action);
		}
		_minimizedDogfights = minimizedDogfightsCount();
	}
}

/**
 * Updates the timer display and resets the palette
 * since it's bound to change on other screens.
 */
void GeoscapeState::init()
{
	State::init();
	timeDisplay();
	updateSlackingIndicator();

	_globe->onMouseClick((ActionHandler)&GeoscapeState::globeClick);
	_globe->onMouseOver(0);
	_globe->rotateStop();
	_globe->setFocus(true);
	_globe->draw();

	// Pop up save screen if it's a new ironman game
	if (getGame()->getSavedGame()->isIronman() && getGame()->getSavedGame()->getName().empty())
	{
		popup(new ListSaveState(OPT_GEOSCAPE));
	}

	// Set music if it's not already playing
	if (_dogfights.empty() && !_dogfightStartTimer->isRunning())
	{
		if (getGame()->getSavedGame()->getMonthsPassed() == -1)
		{
			getGame()->getMod()->playMusic("GMGEO", 1);
		}
		else
		{
			getGame()->getMod()->playMusic("GMGEO");
		}
	}
	else
	{
		getGame()->getMod()->playMusic("GMINTER");
	}
	_globe->setNewBaseHover(false);

	auto baseList = getRegistry().list<Base>();

	for (const Base& xcomBase : getRegistry().list<Base>())
		// run once
	if (getGame()->getSavedGame()->getMonthsPassed() == -1 &&
		// as long as there's a base
		!baseList.empty() &&
		// and it has a name (THIS prevents it from running prior to the base being placed.)
		!baseList.front().getName().empty())
	{
		getGame()->getSavedGame()->addMonth();
		determineAlienMissions();
		// set initial funds. Initial funds are equal to initial funds my facility mainteance and craft maintiance.
		int64_t funds = getGame()->getSavedGame()->getFunds();
		funds -= getRegistry().totalBy<Base, int64_t>(&Base::getFacilityMaintenance, &Base::getCraftMaintenance);
		getGame()->getSavedGame()->setFunds(funds);
	}
}

/**
 * Runs the game timer and handles popups.
 */
void GeoscapeState::update()
{
	State::update();

	_zoomInEffectTimer->think(true, false);
	_zoomOutEffectTimer->think(true, false);
	_dogfightStartTimer->think(true, false);

	if (_popups.empty() && _dogfights.empty() && (!_zoomInEffectTimer->isRunning() || _zoomInEffectDone) && (!_zoomOutEffectTimer->isRunning() || _zoomOutEffectDone))
	{
		// Handle timers
		_gameTimer->think(true, false);
	}
	else
	{
		if (!_dogfights.empty() || _minimizedDogfights != 0)
		{
			// If all dogfights are minimized rotate the globe, etc.
			if (_dogfights.size() == _minimizedDogfights)
			{
				_pause = false;
				_gameTimer->think(true, false);
			}
			_dogfightTimer->think(true, false);
		}
		if (!_popups.empty())
		{
			// Handle popups
			_globe->rotateStop();
			getGame()->pushState(_popups.front());
			_popups.erase(_popups.begin());
		}
	}
}

/**
 * Updates the Geoscape clock with the latest
 * game time and date in human-readable format. (+Funds)
 */
void GeoscapeState::timeDisplay()
{
	if (Options::showFundsOnGeoscape)
	{
		_txtFunds->setText(Unicode::formatFunding(getGame()->getSavedGame()->getFunds()));
	}

	std::ostringstream ss;
	ss << std::setfill('0') << std::setw(2) << getGame()->getSavedGame()->getTime()->getSecond();
	_txtSec->setText(ss.str());

	std::ostringstream ss2;
	ss2 << std::setfill('0') << std::setw(2) << getGame()->getSavedGame()->getTime()->getMinute();
	_txtMin->setText(ss2.str());

	std::ostringstream ss3;
	ss3 << getGame()->getSavedGame()->getTime()->getHour();
	_txtHour->setText(ss3.str());

	std::ostringstream ss4;
	ss4 << getGame()->getSavedGame()->getTime()->getDayString(getGame()->getLanguage());
	_txtDay->setText(ss4.str());

	_txtWeekday->setText(tr(getGame()->getSavedGame()->getTime()->getWeekdayString()));

	_txtMonth->setText(tr(getGame()->getSavedGame()->getTime()->getMonthString()));

	std::ostringstream ss5;
	ss5 << getGame()->getSavedGame()->getTime()->getYear();
	_txtYear->setText(ss5.str());
}

/**
 * Advances the game timer according to
 * the timer speed set, and calls the respective
 * triggers. The timer always advances in "5 secs"
 * cycles, regardless of the speed, otherwise it might
 * skip important steps. Instead, it just keeps advancing
 * the timer until the next speed step (eg. the next day
 * on 1 Day speed) or until an event occurs, since updating
 * the screen on each step would become cumbersomely slow.
 */
void GeoscapeState::timeAdvance()
{
	int timeSpan = 0;
	if (_timeSpeed == _btn5Secs)
	{
		if (Options::oxceGeoSlowdownFactor > 1)
		{
			_slowdownCounter--;
			if (_slowdownCounter > 0)
			{
				// wait
				_globe->draw();
				return;
			}
			else
			{
				_slowdownCounter = Clamp(Options::oxceGeoSlowdownFactor, 2, 100);
			}
		}
		timeSpan = 1;
	}
	else if (_timeSpeed == _btn1Min)
	{
		timeSpan = 12;
	}
	else if (_timeSpeed == _btn5Mins)
	{
		timeSpan = 12 * 5;
	}
	else if (_timeSpeed == _btn30Mins)
	{
		timeSpan = 12 * 5 * 6;
	}
	else if (_timeSpeed == _btn1Hour)
	{
		timeSpan = 12 * 5 * 6 * 2;
	}
	else if (_timeSpeed == _btn1Day)
	{
		timeSpan = 12 * 5 * 6 * 2 * 24;
	}


	for (int i = 0; i < timeSpan && !_pause; ++i)
	{
		TimeTrigger trigger;
		trigger = getGame()->getSavedGame()->getTime()->advance();
		switch (trigger)
		{
		case TIME_1MONTH:
			time1Month();
			FALLTHROUGH;
		case TIME_1DAY:
			time1Day();
			FALLTHROUGH;
		case TIME_1HOUR:
			time1Hour();
			FALLTHROUGH;
		case TIME_30MIN:
			time30Minutes();
			FALLTHROUGH;
		case TIME_10MIN:
			time10Minutes();
			FALLTHROUGH;
		case TIME_5SEC:
			time5Seconds();
		}
	}

	_pause = !_dogfightsToBeStarted.empty() || _zoomInEffectTimer->isRunning() || _zoomOutEffectTimer->isRunning();

	timeDisplay();
	_globe->draw();
}

/**
 * Update list of active crafts.
 * @return Const pointer to updated list.
 */
const std::vector<Craft*>& GeoscapeState::updateActiveCrafts()
{
	_activeCrafts.clear();
	for (const Base& xcomBase : getRegistry().list<Base>())
	{
		for (Craft* xcraft : xcomBase.getCrafts())
		{
			if (xcraft->getStatus() == "STR_OUT" && !xcraft->isDestroyed())
			{
				_activeCrafts.push_back(xcraft);
			}
		}
	}
	return _activeCrafts;
}

/**
 * Takes care of any game logic that has to
 * run every game second, like craft movement.
 */
void GeoscapeState::time5Seconds()
{
	// broadcast game.battlescape.PauseState to any listeners
	//if (!getGame()->getGameScript()->getGeoscapeScript().Broadcast("PauseState"))
	//{
	//	return;
	//}

	// If in "slow mode", handle UFO hunting and escorting logic every 5 seconds, not only every 10 minutes
	if ((_timeSpeed == _btn5Secs || _timeSpeed == _btn1Min) && getGame()->getMod()->getHunterKillerFastRetarget())
	{
		ufoHuntingAndEscorting();
	}

	// Game over if there are no more bases.
	if (getRegistry().empty<Base>())
	{
		getGame()->getSavedGame()->setEnding(END_LOSE);
	}
	if (getGame()->getSavedGame()->getEnding() == END_LOSE)
	{
		getGame()->pushState(new CutsceneState(getGame()->getMod()->getLoseDefeatCutscene()));
		if (getGame()->getSavedGame()->isIronman())
		{
			getGame()->pushState(new SaveGameState(OPT_GEOSCAPE, SAVE_IRONMAN, _palette));
		}
		return;
	}

	const auto& activeCrafts = updateActiveCrafts();

	// Handle UFO logic
	bool ufoIsAttacking = false;
	for (Ufo& ufo : getRegistry().list<Ufo>())
	{
		switch (ufo.getStatus())
		{
		case Ufo::FLYING:
			ufo.think();
			if (ufo.reachedDestination() && !ufo.isEscorting())
			{
				Craft* c = dynamic_cast<Craft*>(ufo.getDestination());
				if (c && !c->isDestroyed() && ufo.isHunting())
				{
					// Check if some other HK already attacked before us (at the very same moment)
					int hkDogfights = 0;
					for (auto f : _dogfights) if (f->isUfoAttacking()) { hkDogfights++; }
					for (auto g : _dogfightsToBeStarted) if (g->isUfoAttacking()) { hkDogfights++; }

					// If yes, wait... not more than 1 HK interception allowed at a time.
					if (hkDogfights >= 1)
					{
						continue;
					}

					// If not, interrupt all other (regular) interceptions to prevent a dead-lock (and other possible side effects)
					for (auto* f : _dogfights) if (f->getCraft()) { f->getCraft()->setInDogfight(false); f->getCraft()->setInterceptionOrder(0); }
					for (auto* g : _dogfightsToBeStarted) if (g->getCraft()) { g->getCraft()->setInDogfight(false); g->getCraft()->setInterceptionOrder(0); }
					Collections::deleteAll(_dogfights);
					Collections::deleteAll(_dogfightsToBeStarted);
					_minimizedDogfights = 0;

					// Start the dogfight
					{
						// Main target
						_dogfightsToBeStarted.push_back(new DogfightState(this, c, &ufo, true));

						// Start fighting escorts and other craft as well (if they are in escort range)
						if (getGame()->getMod()->getEscortsJoinFightAgainstHK())
						{
							int secondaryTargets = 0;
							for (Craft* craft : activeCrafts)
							{
								if (!craft->isIgnoredByHK() && craft != c)
								{
									// craft is close enough and has at least one loaded weapon
									if (craft->getNumWeapons(true) > 0 && craft->getDistance(c) < Nautical(getGame()->getMod()->getEscortRange()))
									{
										// only up to 4 dogfights = 1 main + 3 secondary
										if (secondaryTargets < 3)
										{
											// Note: push_front() is used so that main target is attacked first
											_dogfightsToBeStarted.push_front(new DogfightState(this, craft, &ufo, true));
											secondaryTargets++;
										}
									}
								}
							}
						}

						if (!_dogfightStartTimer->isRunning())
						{
							_pause = true;
							timerReset();
							_globe->center(c->getLongitude(), c->getLatitude());
							startDogfight();
							_dogfightStartTimer->start();
						}
						getGame()->getMod()->playMusic("GMINTER");

						// Don't process certain craft logic (moving and reaching destination)
						ufoIsAttacking = true;
					}
					// Don't handle other logic for this UFO, just continue with the next one
					continue;
				}

				size_t count = getGame()->getSavedGame()->getMissionSites().size();
				AlienMission *mission = ufo.getMission();
				bool detected = ufo.getDetected();
				mission->ufoReachedWaypoint(ufo, *getGame(), *_globe);
				if (Options::oxceUfoLandingAlert && ufo.getStatus() == Ufo::LANDED && ufo.getDetected() && ufo.getLandId() != 0)
				{
					std::string msg = tr("STR_UFO_HAS_LANDED").arg(ufo.getName(getGame()->getLanguage()));
					popup(new CraftErrorState(this, msg));
				}
				if (detected != ufo.getDetected() && !ufo.getFollowers()->empty())
				{
					if (!(ufo.getTrajectory().getID() == UfoTrajectory::RETALIATION_ASSAULT_RUN && ufo.getStatus() == Ufo::LANDED))
						popup(new UfoLostState(ufo.getName(getGame()->getLanguage())));
				}
				if (count < getGame()->getSavedGame()->getMissionSites().size())
				{
					MissionSite *site = getGame()->getSavedGame()->getMissionSites().back();
					site->setDetected(true);
					popup(new MissionDetectedState(site, this));
				}
				// If UFO was destroyed, don't spawn missions
				if (ufo.getStatus() == Ufo::DESTROYED)
					return;
				if (Base *base = dynamic_cast<Base*>(ufo.getDestination()))
				{
					mission->setWaveCountdown(30 * (static_cast<size_t>(RNG::generate(0, 400)) + 48));
					ufo.setDestination(0);
					base->setupDefenses(mission);
					timerReset();
					if (!base->getDefenses().empty() && !ufo.getMission()->getRules().ignoreBaseDefenses())
					{
						popup(new BaseDefenseState(base, &ufo, this));
						return; // don't allow multiple simultaneous attacks in the same game tick
					}
					else
					{
						handleBaseDefense(base, &ufo);
						return;
					}
				}
			}
			// Init UFO shields
			if (ufo.getShield() == -1)
			{
				ufo.setShield(ufo.getCraftStats().shieldCapacity);
			}
			// Recharge UFO shields
			else if (ufo.getShield() < ufo.getCraftStats().shieldCapacity)
			{
				int shieldRechargeInGeoscape = ufo.getCraftStats().shieldRechargeInGeoscape;
				if (shieldRechargeInGeoscape == -1)
				{
					ufo.setShield(ufo.getCraftStats().shieldCapacity);
				}
				else if (shieldRechargeInGeoscape > 0)
				{
					int total = shieldRechargeInGeoscape / 100;
					if (RNG::percent(shieldRechargeInGeoscape % 100))
						total++;
					ufo.setShield(ufo.getShield() + total);
				}
			}
			break;
		case Ufo::LANDED:
			ufo.think();
			if (ufo.getSecondsRemaining() == 0)
			{
				AlienMission *mission = ufo.getMission();
				bool detected = ufo.getDetected();
				mission->ufoLifting(ufo, *getGame()->getSavedGame());
				if (detected != ufo.getDetected() && !ufo.getFollowers()->empty())
				{
					popup(new UfoLostState(ufo.getName(getGame()->getLanguage())));
				}
			}
			break;
		case Ufo::CRASHED:
			ufo.think();
			if (ufo.getSecondsRemaining() == 0)
			{
				ufo.setDetected(false);
				ufo.setStatus(Ufo::DESTROYED);
			}
			break;
		case Ufo::DESTROYED:
			// Nothing to do
			break;
		}
	}

	// Handle craft logic
	for (Base& xcomBase : getRegistry().list<Base>())
	{
		for (auto craftIt = xcomBase.getCrafts().begin(); craftIt != xcomBase.getCrafts().end();)
		{
			Craft* xcraft = (*craftIt);
			if (xcraft->isDestroyed())
			{
				AreaSystem::addXcomActivityToCountryAndRegion(-xcraft->getRules()->getScore(), *xcraft);
				//if (_ufoIsAttacking)
				{
					// Note: this was moved from DogfightState.cpp, as it was not 100% reliable there
					xcraft->evacuateCrew(getGame()->getMod());
				}
				// if a transport craft has been shot down, kill all the soldiers on board.
				if (xcraft->getRules()->getMaxUnitsLimit() > 0)
				{
					for (auto soldierIt = xcomBase.getSoldiers().begin(); soldierIt != xcomBase.getSoldiers().end();)
					{
						Soldier* soldier = (*soldierIt);
						if (soldier->getCraft() == xcraft)
						{
							soldierIt = getGame()->getSavedGame()->killSoldier(true, soldier);
						}
						else
						{
							++soldierIt;
						}
					}
				}
				getGame()->getSavedGame()->stopHuntingXcomCraft(xcraft); // craft destroyed in dogfight
				Craft *craft = *craftIt;
				craftIt = xcomBase.removeCraft(craft, false);
				delete craft;
				continue;
			}
			if (xcraft->getDestination())
			{
				Ufo* u = dynamic_cast<Ufo*>(xcraft->getDestination());
				if (u != 0)
				{
					if (!u->getDetected())
					{
						if (u->getTrajectory().getID() == UfoTrajectory::RETALIATION_ASSAULT_RUN && (u->getStatus() == Ufo::LANDED || u->getStatus() == Ufo::DESTROYED))
						{
							xcraft->returnToBase();
						}
						else
						{
							Waypoint *w = new Waypoint();
							w->setLongitude(u->getLongitude());
							w->setLatitude(u->getLatitude());
							w->setId(u->getId());
							xcraft->setDestination(0);
							popup(new GeoscapeCraftState(xcraft, _globe, w, false));
						}
					}
					if (u->getStatus() == Ufo::LANDED && xcraft->isInDogfight())
					{
						xcraft->setInDogfight(false);
					}
					else if (u->getStatus() == Ufo::DESTROYED)
					{
						xcraft->returnToBase();
					}
				}
				else
				{
					if (xcraft->isInDogfight())
					{
						xcraft->setInDogfight(false);
					}
				}
			}

			if (!ufoIsAttacking)
			{
				bool returnedToBase = xcraft->think();
				if (returnedToBase)
				{
					getGame()->getSavedGame()->stopHuntingXcomCraft(xcraft); // hiding in the base is good enough, obviously
				}
			}

			// Handle craft shield recharge
			if (!ufoIsAttacking && xcraft->getShield() < xcraft->getCraftStats().shieldCapacity)
			{
				int shieldRechargeInGeoscape = xcraft->getCraftStats().shieldRechargeInGeoscape;
				if (shieldRechargeInGeoscape == -1)
				{
					xcraft->setShield(xcraft->getCraftStats().shieldCapacity);
				}
				else if (shieldRechargeInGeoscape > 0)
				{
					int total = xcraft->getCraftStats().shieldRechargeInGeoscape / 100;
					if (RNG::percent(xcraft->getCraftStats().shieldRechargeInGeoscape % 100))
						total++;
					xcraft->setShield(xcraft->getShield() + total);
				}
			}

			if (!ufoIsAttacking && xcraft->reachedDestination())
			{
				Ufo* u = dynamic_cast<Ufo*>(xcraft->getDestination());
				Waypoint *w = dynamic_cast<Waypoint*>(xcraft->getDestination());
				MissionSite* m = dynamic_cast<MissionSite*>(xcraft->getDestination());
				AlienBase* b = dynamic_cast<AlienBase*>(xcraft->getDestination());
				Craft* x = dynamic_cast<Craft*>(xcraft->getDestination());
				if (u != 0)
				{
					switch (u->getStatus())
					{
					case Ufo::FLYING:
						// Not more than 4 interceptions at a time... but hunter-killers are always allowed
						if (!u->isHunterKiller() && _dogfights.size() + _dogfightsToBeStarted.size() >= 4)
						{
							++craftIt;
							continue;
						}
						// Can we actually fight it
						if (!xcraft->isInDogfight() && u->getSpeed() <= xcraft->getCraftStats().speedMax)
						{
							if (u->isHunterKiller())
							{
								// Check if some other HK already attacked before us (at the very same moment)
								int hkDogfights = 0;
								for (auto f : _dogfights) if (f->isUfoAttacking()) { hkDogfights++; }
								for (auto g : _dogfightsToBeStarted) if (g->isUfoAttacking()) { hkDogfights++; }

								// If yes, wait... not more than 1 HK interception allowed at a time.
								if (hkDogfights >= 1)
								{
									++craftIt;
									continue;
								}

								// If not, interrupt all other (regular) interceptions to prevent a dead-lock (and other possible side effects)
								for (auto* f : _dogfights) if (f->getCraft()) { f->getCraft()->setInDogfight(false); f->getCraft()->setInterceptionOrder(0); }
								for (auto* g : _dogfightsToBeStarted) if (g->getCraft()) { g->getCraft()->setInDogfight(false); g->getCraft()->setInterceptionOrder(0); }
								Collections::deleteAll(_dogfights);
								Collections::deleteAll(_dogfightsToBeStarted);
								_minimizedDogfights = 0;

								// Don't process certain craft logic (moving and reaching destination)
								ufoIsAttacking = true;
							}

							// Main target
							DogfightState* dogfight = new DogfightState(this, xcraft, u, u->isHunterKiller());
							_dogfightsToBeStarted.push_back(dogfight);

							if (u->isHunterKiller() && getGame()->getMod()->getEscortsJoinFightAgainstHK())
							{
								// Start fighting escorts and other craft as well (if they are in escort range)
								int secondaryTargets = 0;
								for (Craft* craft : activeCrafts)
								{
									if (!craft->isIgnoredByHK() && craft != xcraft)
									{
										// craft is close enough and has at least one loaded weapon
										if (craft->getNumWeapons(true) > 0 && craft->getDistance(xcraft) < Nautical(getGame()->getMod()->getEscortRange()))
										{
											// only up to 4 dogfights = 1 main + 3 secondary
											if (secondaryTargets < 3)
											{
												// Note: push_front() is used so that main target is attacked first
												_dogfightsToBeStarted.push_front(new DogfightState(this, craft, u, u->isHunterKiller()));
												secondaryTargets++;
											}
										}
									}
								}
							}

							// Ignore these restrictions when fighting against a HK, otherwise it's very easy to avoid being attacked
							if (!u->isHunterKiller())
							{
								if (xcraft->getRules()->isWaterOnly() && u->getAltitudeInt() > xcraft->getRules()->getMaxAltitude())
								{
									popup(new DogfightErrorState(xcraft, tr("STR_UNABLE_TO_ENGAGE_DEPTH")));
									dogfight->setMinimized(true);
									dogfight->setWaitForAltitude(true);
								}
								else if (xcraft->getRules()->isWaterOnly() && !_globe->insideLand(xcraft->getLongitude(), xcraft->getLatitude()))
								{
									popup(new DogfightErrorState(xcraft, tr("STR_UNABLE_TO_ENGAGE_AIRBORNE")));
									dogfight->setMinimized(true);
									dogfight->setWaitForPoly(true);
								}
							}

							if (!_dogfightStartTimer->isRunning())
							{
								_pause = true;
								timerReset();
								_globe->center(xcraft->getLongitude(), xcraft->getLatitude());
								startDogfight();
								_dogfightStartTimer->start();
							}
							getGame()->getMod()->playMusic("GMINTER");
						}
						break;
					case Ufo::LANDED:
					case Ufo::CRASHED:
					case Ufo::DESTROYED: // Just before expiration
						if (xcraft->getNumTotalUnits() > 0 && xcraft->getRules()->getAllowLanding())
						{
							if (!xcraft->isInDogfight())
							{
								// look up polygons texture
								int texture, shade;
								_globe->getPolygonTextureAndShade(u->getLongitude(), u->getLatitude(), &texture, &shade);
								timerReset();
								Texture* globeTexture = getGame()->getMod()->getGlobe()->getTexture(texture);
								popup(new ConfirmLandingState(xcraft, globeTexture, globeTexture, shade));
							}
						}
						else if (u->getStatus() != Ufo::LANDED)
						{
							xcraft->returnToBase();
						}
						break;
					}
				}
				else if (w != 0)
				{
					if (!xcraft->getIsAutoPatrolling())
					{
						popup(new CraftPatrolState(xcraft, _globe));
					}
					xcraft->setDestination(0);
				}
				else if (m != 0)
				{
					if (xcraft->getNumTotalUnits() > 0 && xcraft->getRules()->getAllowLanding())
					{
						// look up polygons texture
						int texture, shade;
						_globe->getPolygonTextureAndShade(m->getLongitude(), m->getLatitude(), &texture, &shade);
						timerReset();
						auto globeTexture = getGame()->getMod()->getGlobe()->getTexture(texture);
						auto missionTexture = getGame()->getMod()->getGlobe()->getTexture(m->getTexture());
						if (!missionTexture)
						{
							missionTexture = globeTexture;
						}
						popup(new ConfirmLandingState(xcraft, missionTexture, globeTexture, shade));
					}
					else
					{
						xcraft->returnToBase();
					}
				}
				else if (b != 0)
				{
					if (b->isDiscovered())
					{
						if (xcraft->getNumTotalUnits() > 0 && xcraft->getRules()->getAllowLanding())
						{
							int texture, shade;
							_globe->getPolygonTextureAndShade(b->getLongitude(), b->getLatitude(), &texture, &shade);
							timerReset();
							Texture* globeTexture = getGame()->getMod()->getGlobe()->getTexture(texture);
							popup(new ConfirmLandingState(xcraft, globeTexture, globeTexture, shade));
						}
						else
						{
							xcraft->returnToBase();
						}
					}
				}
				else if (x != 0)
				{
					if (x->getStatus() != "STR_OUT" || x->isDestroyed())
					{
						xcraft->returnToBase();
					}
				}
			}
			 ++craftIt;
		}
	}

	// Clean up dead UFOs and end dogfights which were minimized.
	auto destroyedUfoFilter = [](auto tuple) { return std::get<1>(tuple).getStatus() == Ufo::DESTROYED; };
	for (auto&& [id, ufo] : getRegistry().raw().view<Ufo>().each() | std::views::filter(destroyedUfoFilter))
	{
		Collections::deleteIf(_dogfights, _dogfights.size(), [&](DogfightState* dogfight) { return dogfight->getUfo() == &ufo; });
		getRegistry().raw().destroy(id);
	}

	// Check any dogfights waiting to open
	for (auto* dfs : _dogfights)
	{
		if (dfs->isMinimized())
		{
			if ((dfs->getWaitForPoly() && _globe->insideLand(dfs->getUfo()->getLongitude(), dfs->getUfo()->getLatitude())) ||
				(dfs->getWaitForAltitude() && dfs->getUfo()->getAltitudeInt() <= dfs->getCraft()->getRules()->getMaxAltitude()))
			{
				_pause = true; // the USO reached the sea during this interval period, stop the timer and let handleDogfights() take it from there.
				timerReset();
			}
		}
	}

	// Clean up unused waypoints
	Collections::deleteIf(getGame()->getSavedGame()->getWaypoints(), getGame()->getSavedGame()->getWaypoints().size(),
		[&](Waypoint* way)
		{
			return way->getFollowers()->empty();
		}
	);
}

/**
 * Takes care of any game logic that has to
 * run every game ten minutes, like fuel consumption.
 */
void GeoscapeState::time10Minutes()
{
	for (Base& xcomBase : getRegistry().list<Base>())
	{
		// Fuel consumption for XCOM craft.
		for (Craft* xcraft : xcomBase.getCrafts())
		{
			if (xcraft->getStatus() == "STR_OUT")
			{
				int escortSpeed = 0;
				{
					Craft *escortee = dynamic_cast<Craft*>(xcraft->getDestination());
					if (escortee && xcraft->getDistance(escortee) < Nautical(getGame()->getMod()->getEscortRange()))
					{
						escortSpeed = escortee->getSpeed();
					}
				}
				xcraft->consumeFuel(escortSpeed);
				if (!xcraft->getLowFuel() && xcraft->getFuel() <= xcraft->getFuelLimit())
				{
					xcraft->setLowFuel(true);
					xcraft->returnToBase();
					if (!xcraft->getIsAutoPatrolling())
					{
						popup(new LowFuelState(xcraft, this));
					}
				}

				if (xcraft->getDestination() == 0 && xcraft->getCraftStats().sightRange > 0)
				{
					double range = Nautical(xcraft->getCraftStats().sightRange);
					for (AlienBase* ab : getGame()->getSavedGame()->getAlienBases())
					{
						if (xcraft->getDistance(ab) <= range
							&& RNG::percent(static_cast<int>(50-(xcraft->getDistance(ab) / range) * 50))
							&& !ab->isDiscovered())
						{
							ab->setDiscovered(true);
						}
					}
				}
			}
		}
	}

	if (Options::aggressiveRetaliation)
	{
		// Detect as many bases as possible.
		for (Base& xcomBase : getRegistry().list<Base>())
		{
			// Note all UFO Mission types detect on AggressiveRetaliation
			auto xcomBaseDetector = [&xcomBase](const Ufo& ufo) {
				if (ufo.getTrajectoryPoint() <= 1) return false;
				if (ufo.getTrajectory().getZone(ufo.getTrajectoryPoint()) == 5) return false;
				// UFOs attacking a base don't detect!
				if (ufo.getTrajectory().getID() == UfoTrajectory::RETALIATION_ASSAULT_RUN) return false;
				if (ufo.isCrashed()) return false;
				// UFOs have a detection range of 80 XCOM units. - we use a great circle formula and nautical miles.
				if (xcomBase.getDistance(&ufo) >= Nautical(ufo.getCraftStats().sightRange)) return false;
				return RNG::percent((int)xcomBase.getDetectionChance());
			};

			// Find a UFO that detected this base, if any.
			if (getRegistry().findValue_if<Ufo>(xcomBaseDetector)) { xcomBase.setRetaliationTarget(true); }
		}
	}
	else
	{
		// Only remember last base in each region.
		std::unordered_map<entt::entity, entt::entity> discovered;
		for (const auto&& [baseId, xcomBase] : getRegistry().raw().view<Base>().each())
		{
			auto xcomBaseDetector = [&xcomBase](const Ufo& ufo) {
				if (ufo.getTrajectoryPoint() <= 1) return false;
				if (ufo.getTrajectory().getZone(ufo.getTrajectoryPoint()) == 5) return false;
				// only UFOs on retaliation missions actively scan for bases
				if ((ufo.getMission()->getRules().getObjective() != OBJECTIVE_RETALIATION)) return false;
				// UFOs attacking a base don't detect!
				if (ufo.getTrajectory().getID() == UfoTrajectory::RETALIATION_ASSAULT_RUN) return false;
				if (ufo.isCrashed()) return false;
				// UFOs have a detection range of 80 XCOM units. - we use a great circle formula and nautical miles.
				if (xcomBase.getDistance(&ufo) >= Nautical(ufo.getCraftStats().sightRange)) return false;
				return RNG::percent((int)xcomBase.getDetectionChance());
			};

			// Find a UFO that detected this base, if any.
			if (getRegistry().findValue_if<const Ufo>(xcomBaseDetector))
			{
				entt::entity regionId = AreaSystem::locate<Region>(xcomBase);
				discovered[regionId] = baseId;
			}
		}
		// Now mark the bases as discovered.
		for (const std::pair<entt::entity, entt::entity> pair : discovered)
		{
			getRegistry().raw().get<Base>(pair.second).setRetaliationTarget(true);
		}
	}

	// Handle alien bases detecting xcom craft and generating hunt missions
	baseHunting();

	// Handle UFO re-targeting (i.e. hunting and escorting) logic
	ufoHuntingAndEscorting();
}

void GeoscapeState::ufoHuntingAndEscorting()
{
	auto& activeCrafts = updateActiveCrafts();

	auto ufoFilter = [](const Ufo& ufo) { return ufo.isHunterKiller() && ufo.getStatus() == Ufo::FLYING; };
	for (Ufo& ufo : getRegistry().list<Ufo>(ufoFilter))
	{
		// current target and attraction
		int newAttraction = INT_MAX;
		Craft *newTarget = nullptr;
		Craft *originalTarget = nullptr;
		if (ufo.isHunting())
		{
			originalTarget = ufo.getTargetedXcomCraft();
		}
		if (originalTarget && !originalTarget->isIgnoredByHK() && ufo.insideRadarRange(originalTarget))
		{
			newTarget = originalTarget;
			newAttraction = newTarget->getHunterKillerAttraction(ufo.getHuntMode());
		}

		// look for more attractive target
		for (Craft* craft : activeCrafts)
		{
			if (!craft->isIgnoredByHK() && !craft->getRules()->isUndetectable())
			{
				int tmpAttraction = craft->getHunterKillerAttraction(ufo.getHuntMode());
				if (tmpAttraction < newAttraction && ufo.insideRadarRange(craft))
				{
					newTarget = craft;
					newAttraction = tmpAttraction;
				}
			}
		}

		if (newTarget && newTarget != originalTarget)
		{
			// set new target
			ufo.setTargetedXcomCraft(newTarget);
			// TODO: rethink: always reveal the hunting UFO (even outside of radar range?)
			ufo.setDetected(true);
			if (ufo.getId() == 0)
			{
				ufo.setId(getGame()->getSavedGame()->getId("STR_UFO"));
			}
			// inform the player
			if (ufo.getRules()->getHuntAlertSound() != Mod::NO_SOUND)
			{
				getGame()->getMod()->getSound("GEO.CAT", ufo.getRules()->getHuntAlertSound())->play();
			}
			std::string msg = tr("STR_UFO_STARTED_HUNTING")
				.arg(ufo.getName(getGame()->getLanguage()))
				.arg(newTarget->getName(getGame()->getLanguage()));
			popup(new CraftErrorState(this, msg));
		}
		else if (originalTarget)
		{
			// stop hunting
			ufo.resetOriginalDestination(originalTarget);
		}

		// If we are not preoccupied by hunting, let's see if there is still anyone left to escort
		if (ufo.isEscort() && !ufo.isHunting() && !ufo.isEscorting())
		{
			auto escortPredicate = [&ufo](const Ufo& escortUfo) {
				return escortUfo.getMission()->getId() == ufo.getMission()->getId() // From the same mission
					&& !escortUfo.isHunterKiller(); // But not another hunter-killer, we escort only normal UFOs
			};
			if (Ufo* escortUfo = getRegistry().findValue_if<Ufo>(escortPredicate))
			{
				ufo.setEscortedUfo(escortUfo);
			}
		}
	}
}

void GeoscapeState::baseHunting()
{
	auto activeCrafts = updateActiveCrafts();

	for (AlienBase* ab : getGame()->getSavedGame()->getAlienBases())
	{
		if (ab->getDeployment()->getBaseDetectionRange() > 0)
		{
			// Increase counter by 10 minutes
			ab->setMinutesSinceLastHuntMissionGeneration(ab->getMinutesSinceLastHuntMissionGeneration() + 10);

			// Check counter
			if (ab->getMinutesSinceLastHuntMissionGeneration() >= ab->getDeployment()->getHuntMissionMaxFrequency())
			{
				// Look for nearby craft
				bool started = false;
				for (Craft* craft : activeCrafts)
				{
					// Craft is flying (i.e. not in base)
					if (craft->getStatus() == "STR_OUT" && !craft->isDestroyed() && !craft->getRules()->isUndetectable() && !craft->isIgnoredByHK())
					{
						// Craft is close enough and RNG is in our favour
						if (craft->getDistance(ab) < Nautical(ab->getDeployment()->getBaseDetectionRange()) && RNG::percent(ab->getDeployment()->getBaseDetectionChance()))
						{
							// Generate a hunt mission
							const std::string huntMission = ab->getDeployment()->generateHuntMission(getGame()->getSavedGame()->getMonthsPassed());
							if (getGame()->getMod()->getAlienMission(huntMission))
							{
								// Spawn hunt mission for this base.
								const RuleAlienMission &rule = *getGame()->getMod()->getAlienMission(huntMission);
								AlienMission *mission = new AlienMission(rule);
								mission->setRegion(AreaSystem::locateValue<Region>(*ab)->getRules()->getType(), *getGame()->getMod());
								mission->setId(getGame()->getSavedGame()->getId("ALIEN_MISSIONS"));
								if (!ab->getDeployment()->isHuntMissionRaceFromAlienBase() && rule.hasRaceWeights())
								{
									mission->setRace(rule.generateRace(getGame()->getSavedGame()->getMonthsPassed()));
								}
								else
								{
									mission->setRace(ab->getAlienRace());
								}
								mission->setAlienBase(ab);
								int targetArea = -1;
								if (mission->getRules().getObjective() == OBJECTIVE_SITE)
								{
									int missionZone = mission->getRules().getSpawnZone();
									RuleRegion *regionRules = getGame()->getMod()->getRegion(mission->getRegion());
									const std::vector<MissionArea> areas = regionRules->getMissionZones().at(missionZone).areas;
									if (!areas.empty())
									{
										targetArea = RNG::generate(0, (int)areas.size() - 1);
									}
								}
								mission->setMissionSiteZoneArea(targetArea);
								mission->start(*getGame(), *_globe);
								getGame()->getSavedGame()->getAlienMissions().push_back(mission);

								// Start immediately
								mission->think(*getGame(), *_globe);

								// Reset counter
								ab->setMinutesSinceLastHuntMissionGeneration(0);
								started = true;
								break;
							}
							else if (huntMission != "")
							{
								throw Exception("Alien Base tried to generate undefined hunt mission: " + huntMission);
							}
						}
					}
					if (started) break;
				}
			}
		}
	}
}

/** @brief Process a MissionSite.
 * This function object will count down towards expiring a MissionSite, and handle expired MissionSites.
 * @param ts Pointer to mission site.
 * @return Has mission site expired?
 */
bool GeoscapeState::processMissionSite(MissionSite *site)
{
	bool removeSite = site->getSecondsRemaining() < 30 * 60;
	if (!removeSite)
	{
		site->setSecondsRemaining(site->getSecondsRemaining() - 30 * 60);
	}
	else
	{
		bool noFollowers = site->getFollowers()->empty();
		if (site->getRules()->despawnEvenIfTargeted())
		{
			for (auto* follower : site->getCraftFollowers())
			{
				follower->returnToBase();
			}
			if (!noFollowers)
			{
				popup(new UfoLostState(site->getName(getGame()->getLanguage())));
			}
		}
		else
		{
			removeSite = noFollowers; // CHEEKY EXPLOIT
		}
	}
	if (removeSite)
	{
		// Unlock research defined in alien deployment, if the mission site despawned
		const RuleResearch* research = getGame()->getMod()->getResearch(site->getDeployment()->getUnlockedResearchOnDespawn());
		getGame()->getSavedGame()->handleResearchUnlockedByMissions(research, getGame()->getMod());

		// Increase counters
		getGame()->getSavedGame()->increaseCustomCounter(site->getDeployment()->getCounterDespawn());
		getGame()->getSavedGame()->increaseCustomCounter(site->getDeployment()->getCounterFailure()); // despawn is also a type of failure
		getGame()->getSavedGame()->increaseCustomCounter(site->getDeployment()->getCounterAll());
		// Decrease counters
		getGame()->getSavedGame()->decreaseCustomCounter(site->getDeployment()->getDecreaseCounterDespawn());
		getGame()->getSavedGame()->decreaseCustomCounter(site->getDeployment()->getDecreaseCounterFailure()); // despawn is also a type of failure
		getGame()->getSavedGame()->decreaseCustomCounter(site->getDeployment()->getDecreaseCounterAll());

		// Generate a despawn event
		auto eventRules = getGame()->getMod()->getEvent(site->getDeployment()->chooseDespawnEvent());
		bool canSpawn = getGame()->getSavedGame()->canSpawnInstantEvent(eventRules);
		if (canSpawn)
		{
			timerReset();
			popup(new GeoscapeEventState(*eventRules));
		}
	}

	int score = removeSite ? site->getDeployment()->getDespawnPenalty() : site->getDeployment()->getPoints();

	AreaSystem::addAlienActivityToCountryAndRegion(score, *site);
	return removeSite;
}

/**
 * Takes care of any game logic that has to
 * run every game half hour, like UFO detection.
 */
void GeoscapeState::time30Minutes()
{
	getRegistry().getService<GeoSystem>().updateAllRegionsAndCountries();

	// Decrease mission countdowns
	for (auto am : getGame()->getSavedGame()->getAlienMissions())
	{
		size_t abCount = getGame()->getSavedGame()->getAlienBases().size();

		am->think(*getGame(), *_globe);

		if (abCount < getGame()->getSavedGame()->getAlienBases().size())
		{
			AlienBase* newAlienBase = getGame()->getSavedGame()->getAlienBases().back();
			if (!newAlienBase->isDiscovered() && am->getRules().showAlienBase())
			{
				newAlienBase->setDiscovered(true);
				popup(new AlienBaseState(newAlienBase, this));
			}
		}

		if (am->getRules().getObjective() == OBJECTIVE_RETALIATION && am->isOver())
		{
			for (Base& xcomBase : getRegistry().list<Base>())
			{
				if (xcomBase.getRetaliationMission() == am)
				{
					xcomBase.setRetaliationMission(nullptr);
				}
			}
		}
	}

	// Remove finished missions
	Collections::deleteIf(
		getGame()->getSavedGame()->getAlienMissions(),
		[](AlienMission* am)
		{
			return am->isOver();
		}
	);

	// Handle crashed UFOs expiration
	auto crashedStatus = [](const Ufo& ufo) { return ufo.getStatus() == Ufo::CRASHED; };
	for (Ufo& ufo : getRegistry().list<Ufo>(crashedStatus))
	{
		if (ufo.getSecondsRemaining() >= 30 * 60)
		{
			ufo.setSecondsRemaining(ufo.getSecondsRemaining() - 30 * 60);
			continue;
		}
		// Marked expired UFOs for removal.
		ufo.setStatus(Ufo::DESTROYED);
	}

	// Handle craft maintenance and alien base detection
	for (Base& xcomBase : getRegistry().list<Base>())
	{
		for (Craft* xcraft : xcomBase.getCrafts())
		{
			if (xcraft->getStatus() == "STR_REFUELLING")
			{
				std::string item = xcraft->refuel();

				if (item.empty())
				{
					// notification
					if (xcraft->getStatus() == "STR_READY" && xcraft->getRules()->notifyWhenRefueled())
					{
						std::string msg = tr("STR_CRAFT_IS_READY").arg(xcraft->getName(getGame()->getLanguage())).arg(xcomBase.getName());
						popup(new CraftErrorState(this, msg));
					}
					// auto-patrol
					if (xcraft->getStatus() == "STR_READY" && xcraft->getRules()->canAutoPatrol())
					{
						if (xcraft->getIsAutoPatrolling())
						{
							Waypoint *w = new Waypoint();
							w->setLongitude(xcraft->getLongitudeAuto());
							w->setLatitude(xcraft->getLatitudeAuto());
							if (w != 0 && w->getId() == 0)
							{
								w->setId(getGame()->getSavedGame()->getId("STR_WAY_POINT"));
								getGame()->getSavedGame()->getWaypoints().push_back(w);
							}
							xcraft->setDestination(w);
							xcraft->setStatus("STR_OUT");
						}
					}
				}
				else
				{
					std::string msg = tr("STR_NOT_ENOUGH_ITEM_TO_REFUEL_CRAFT_AT_BASE")
										.arg(tr(item))
										.arg(xcraft->getName(getGame()->getLanguage()))
										.arg(xcomBase.getName());
					popup(new CraftErrorState(this, msg));
				}
			}
		}
	}

	// can be updated by previous loop
	auto& activeCrafts = updateActiveCrafts();

	// hidden alien activity variables
	entt::entity ufoRegion = entt::null;
	entt::entity ufoCountry = entt::null;

	std::unordered_map<entt::entity, int> hiddenUfoRegions;
	std::unordered_map<entt::entity, int> hiddenUfoCountries;

	// Handle UFO detection and give aliens points
	auto ignoreInstantRetaliation = [](const Ufo& ufo) { // instant retaliation missions are ignored (UFOs shouldn't be detected)
		return ufo.getMission()->getRules().getObjective() != OBJECTIVE_INSTANT_RETALIATION; };
	for (Ufo& ufo : getRegistry().list<Ufo>(ignoreInstantRetaliation))
	{
		int points = ufo.getRules()->getMissionScore(); //one point per UFO in-flight per half hour
		switch (ufo.getStatus())
		{
		case Ufo::LANDED:
			points *= 2;
			FALLTHROUGH;
		case Ufo::FLYING:
			ufoRegion = entt::null;

			AreaSystem::addAlienActivityToCountryAndRegion(points, ufo);

			// detection ufo state
			ufoDetection(&ufo, activeCrafts);

			// accumulate hidden ufos
			if (!ufo.getDetected())
			{
				if (ufoRegion != entt::null)
				{
					hiddenUfoRegions[ufoRegion]++;
				}
				if (ufoCountry != entt::null)
				{
					hiddenUfoCountries[ufoCountry]++;
				}
			}
			break;

		case Ufo::CRASHED:
		case Ufo::DESTROYED:
			break;
		}
	}

	// update hidden alien activity
	if (Options::displayHiddenAlienActivity != 0)
	{
		std::unordered_map<entt::entity, int> displayHiddenAlienActivityRegions;
		std::unordered_map<entt::entity, int> displayHiddenAlienActivityCountries;
		bool displayHiddenAlienActivityPopup = false;

		displayHiddenAlienActivityPopup = processHiddenAlienActivity<Region>(_hiddenAlienActivityRegions,
			hiddenUfoRegions, displayHiddenAlienActivityRegions);
		displayHiddenAlienActivityPopup = processHiddenAlienActivity<Country>(_hiddenAlienActivityCountries,
			hiddenUfoCountries, displayHiddenAlienActivityCountries);

		// display hidden alien activity
		if (displayHiddenAlienActivityPopup)
		{
			popup(new HiddenAlienActivityState(this, *getGame()->getSavedGame(), displayHiddenAlienActivityRegions, displayHiddenAlienActivityCountries));
		}
	}

	// Processes MissionSites
	Collections::deleteIf(getGame()->getSavedGame()->getMissionSites(),
		[&](MissionSite* site) { return processMissionSite(site); }
	);

	// Decrease event countdowns and pop up if needed
	for (GeoscapeEvent* ge : getGame()->getSavedGame()->getGeoscapeEvents())
	{
		ge->think();

		if (ge->isOver())
		{
			bool interrupted = false;
			if (!ge->getRules().getInterruptResearch().empty())
			{
				if (getGame()->getSavedGame()->isResearched(ge->getRules().getInterruptResearch(), false))
				{
					interrupted = true;
				}
			}
			if (!interrupted)
			{
				timerReset();
				popup(new GeoscapeEventState(ge->getRules()));
			}
		}
	}

	// Remove finished events
	Collections::deleteIf(getGame()->getSavedGame()->getGeoscapeEvents(),
		[](GeoscapeEvent *ge) { return ge->isOver(); }
	);
}

template <typename AreaType>
bool GeoscapeState::processHiddenAlienActivity(
	std::unordered_map<entt::entity, int>& hiddenAlienActivityAreas,
	std::unordered_map<entt::entity, int>& hiddenUfoAreas,
	std::unordered_map<entt::entity, int>& displayHiddenAlienActivityAreas)
{
	bool displayHiddenAlienActivityPopup = false;

	for (entt::entity id : getRegistry().raw().view<AreaType>()) {
		// old value
		int oldHiddenAlienActivity = hiddenAlienActivityAreas[id];

		if (hiddenUfoAreas.find(id) != hiddenUfoAreas.end()) {
			// there are hidden UFOs
			hiddenAlienActivityAreas[id] += hiddenUfoAreas[id];

			// check if we reached notification threshold
			if (Options::displayHiddenAlienActivity == 1
				&& oldHiddenAlienActivity < HIDDEN_ALIEN_ACTIVITY_THRESHOLD
				&& hiddenAlienActivityAreas[id] >= HIDDEN_ALIEN_ACTIVITY_THRESHOLD
				|| Options::displayHiddenAlienActivity == 2)
			{
				displayHiddenAlienActivityAreas[id] = hiddenAlienActivityAreas[id];
				displayHiddenAlienActivityPopup = true;
			}
		}
		else {
			// there are no hidden UFOs
			hiddenAlienActivityAreas.erase(id);

			// show information for detailed notification if activity ceased
			if (Options::displayHiddenAlienActivity == 2 && oldHiddenAlienActivity > 0) {
				displayHiddenAlienActivityAreas[id] = 0;
				displayHiddenAlienActivityPopup = true;
			}
		}
	}
	return displayHiddenAlienActivityPopup;
}

/**
 * Logic responsible for detecting ufo and its tracking.
 * @param ufo
 */
void GeoscapeState::ufoDetection(Ufo* ufo, const std::vector<Craft*>& activeCrafts)
{
	auto maskTest = [](UfoDetection value, UfoDetection mask)
	{
		return (value & mask) == mask;
	};
	auto maskBitOr = [](UfoDetection value, UfoDetection mask)
	{
		return (UfoDetection)(value | mask);
	};

	auto detected = DETECTION_NONE;
	auto alreadyTracked = ufo->getDetected();
	auto save = getGame()->getSavedGame();

	for (Base& xcomBase : getRegistry().list<Base>())
	{
		detected = maskBitOr(detected, xcomBase.detect(ufo, save, alreadyTracked));
	}

	for (Craft* craft : activeCrafts)
	{
		detected = maskBitOr(detected, craft->detect(ufo, save, alreadyTracked));
	}

	if (!alreadyTracked)
	{
		if (maskTest(detected, DETECTION_RADAR))
		{
			if (maskTest(detected, DETECTION_HYPERWAVE))
			{
				ufo->setHyperDetected(true);
			}
			ufo->setDetected(true);
			// don't show if player said he doesn't want to see this UFO anymore
			if (!getGame()->getSavedGame()->isUfoOnIgnoreList(ufo->getId()))
			{
				popup(new UfoDetectedState(ufo, this, true, ufo->getHyperDetected()));
			}
		}
	}
	else
	{
		if (maskTest(detected, DETECTION_HYPERWAVE))
		{
			ufo->setHyperDetected(true);
		}
		// TODO: rethink: hunting UFOs stay visible even outside of radar range?
		if (!maskTest(detected, DETECTION_RADAR) && !ufo->isHunting())
		{
			ufo->setDetected(false);
			ufo->setHyperDetected(false);
			if (!ufo->getFollowers()->empty())
			{
				popup(new UfoLostState(ufo->getName(getGame()->getLanguage())));
			}
		}
	}
}

/**
 * Takes care of any game logic that has to
 * run every game hour, like transfers.
 */
void GeoscapeState::time1Hour()
{
	// Handle craft maintenance
	for (Base& xcomBase : getRegistry().list<Base>())
	{
		for (Craft* xcraft : xcomBase.getCrafts())
		{
			if (xcraft->getStatus() == "STR_REPAIRS")
			{
				xcraft->repair();
			}
			else if (xcraft->getStatus() == "STR_REARMING")
			{
				auto* ammo = xcraft->rearm();
				if (ammo)
				{
					std::string msg = tr("STR_NOT_ENOUGH_ITEM_TO_REARM_CRAFT_AT_BASE")
									   .arg(tr(ammo->getType()))
									   .arg(xcraft->getName(getGame()->getLanguage()))
									   .arg(xcomBase.getName());
					popup(new CraftErrorState(this, msg));
				}
			}
			if (xcraft->getShieldCapacity() > 0 && xcraft->getStatus() != "STR_OUT")
			{
				// Recharge craft shields in parallel (no wait for repair/rearm/refuel)
				xcraft->setShield(xcraft->getShield() + xcraft->getRules()->getShieldRechargeAtBase());
			}
		}

		// Handle base defenses maintenance
		for (BaseFacility* facility : xcomBase.getFacilities())
		{
			const RuleItem* ammo = facility->rearm();
			if (ammo)
			{
				std::string msg = tr("STR_NOT_ENOUGH_ITEM_TO_REARM_FACILITY_AT_BASE")
					.arg(tr(ammo->getType()))
					.arg(tr(facility->getRules()->getType()))
					.arg(xcomBase.getName());
				popup(new CraftErrorState(this, msg));
			}
		}

		// Handle transfers
		bool window = false;
		for (Transfer* transfer : xcomBase.getTransfers())
		{
			transfer->advance(&xcomBase);
			if (!window && transfer->getHours() <= 0)
			{
				window = true;
			}
		}
		if (window)
		{
			popup(new ItemsArrivingState(this));
		}

		// Handle Production
		std::map<Production*, productionProgress_e> toRemove;
		for (Production* prod : xcomBase.getProductions())
		{
			toRemove[prod] = prod->step(&xcomBase, getGame()->getSavedGame(), getGame()->getMod(), getGame()->getLanguage());
		}
		for (const std::pair<Production*, productionProgress_e>& pair : toRemove)
		{
			if (pair.second > PROGRESS_NOT_COMPLETE)
			{
				popup(new ProductionCompleteState(&xcomBase, tr(pair.first->getRules()->getName()), this, pair.second, pair.first));
				xcomBase.removeProduction(pair.first);
			}
		}

		if (Options::storageLimitsEnforced)
		{
			if (xcomBase.storesOverfull())
			{
				timerReset();
				popup(new ErrorMessageState(tr("STR_STORAGE_EXCEEDED").arg(xcomBase.getName()), _palette, getGame()->getMod()->getInterface("geoscape")->getElement("errorMessage")->color, "BACK13.SCR", getGame()->getMod()->getInterface("geoscape")->getElement("errorPalette")->color));
				popup(new SellState(&xcomBase, 0));
			}
			if (!getGame()->getSavedGame()->getAlienContainmentChecked())
			{
				std::map<int, int> prisonTypes;
				for (const std::pair<const RuleItem*, int>& item : xcomBase.getStorageItems()->getContents())
				{
					const RuleItem* rule = item.first;
					if (rule->isAlien())
					{
						prisonTypes[rule->getPrisonType()] += 1;
					}
				}
				for (const std::pair<int, int>& pair : prisonTypes)
				{
					int prisonType = pair.first;
					if (xcomBase.getUsedContainment(prisonType) > xcomBase.getAvailableContainment(prisonType))
					{
						timerReset();
						popup(new ErrorMessageState(
							trAlt("STR_CONTAINMENT_EXCEEDED", prisonType).arg(xcomBase.getName()),
							_palette,
							getGame()->getMod()->getInterface("geoscape")->getElement("errorMessage")->color,
							"BACK01.SCR",
							getGame()->getMod()->getInterface("geoscape")->getElement("errorPalette")->color));
						popup(new ManageAlienContainmentState(&xcomBase, prisonType, OPT_GEOSCAPE));
						break;
					}
				}
			}
		}
	}
	getGame()->getSavedGame()->setAlienContainmentChecked(true); // check only once after reload

	bool postpone = false;
	for (MissionSite* msite : getGame()->getSavedGame()->getMissionSites())
	{
		if (!msite->getDetected())
		{
			postpone = true;
			msite->setDetected(true);
			if (msite->getSecondsRemaining() < 3600)
			{
				msite->setSecondsRemaining(3600); // minimum 1 hour since detection
			}
			popup(new MissionDetectedState(msite, this));
			break; // only one popup per hour!
		}
	}
	if (postpone)
	{
		for (MissionSite* msite : getGame()->getSavedGame()->getMissionSites())
		{
			if (!msite->getDetected())
			{
				msite->setSecondsRemaining(msite->getSecondsRemaining() + 3600); // +1 hour
			}
		}
	}

	updateSlackingIndicator();
}

/**
 * This class will attempt to generate a supply mission for a base.
 * Each alien base has a 6/101 chance to generate a supply mission.
 */
class GenerateSupplyMission
{
public:
	/// Store rules and game data references for later use.
	GenerateSupplyMission(Game &engine, const Globe &globe) : _engine(engine), _globe(globe) { /* Empty by design */ }
	/// Check and spawn mission.
	void operator()(AlienBase *base) const;
private:
	Game &_engine;
	const Globe &_globe;
};

/**
 * Check and create supply mission for the given base.
 * There is a 6/101 chance of the mission spawning.
 * @param base A pointer to the alien base.
 */
void GenerateSupplyMission::operator()(AlienBase *base) const
{
	const Mod &_mod = *_engine.getMod();
	SavedGame &_save = *_engine.getSavedGame();

	std::string missionName = base->getDeployment()->chooseGenMissionType();
	if (_mod.getAlienMission(missionName))
	{
		if (base->getGenMissionCount() < base->getDeployment()->getGenMissionLimit() && RNG::percent(base->getDeployment()->getGenMissionFrequency()))
		{
			//Spawn supply mission for this base.
			const RuleAlienMission &rule = *_mod.getAlienMission(missionName);
			AlienMission *mission = new AlienMission(rule);
			std::string targetRegion;
			if (RNG::percent(rule.getTargetBaseOdds()))
			{
				// 1. target a random xcom base region
				std::vector<std::string> regionsWithXcomBases;
				for (Base& xcomBase : getRegistry().list<Base>())
				{
					if (Region* region = AreaSystem::locateValue<Region>(xcomBase))
					{
						regionsWithXcomBases.push_back(region->getRules()->getType());
					}
				}
				int randomIndex = RNG::generate(0, static_cast<int>(regionsWithXcomBases.size()) - 1);
				targetRegion = regionsWithXcomBases[randomIndex];
			}
			else if (rule.hasRegionWeights())
			{
				// 2. target one of the defined (weighted) regions
				targetRegion = rule.generateRegion(_save.getMonthsPassed());
			}
			else
			{
				// 3. target the region of the alien base (vanilla default)
				targetRegion = AreaSystem::locateValue<Region>(*base)->getRules()->getType();
			}
			mission->setRegion(targetRegion, _mod);
			mission->setId(_save.getId("ALIEN_MISSIONS"));
			if (!base->getDeployment()->isGenMissionRaceFromAlienBase() && rule.hasRaceWeights())
			{
				mission->setRace(rule.generateRace(_save.getMonthsPassed()));
			}
			else
			{
				mission->setRace(base->getAlienRace());
			}
			mission->setAlienBase(base);
			int targetArea = -1;
			if (mission->getRules().getObjective() == OBJECTIVE_SITE)
			{
				int missionZone = mission->getRules().getSpawnZone();
				RuleRegion *regionRules = _mod.getRegion(mission->getRegion());
				const std::vector<MissionArea> areas = regionRules->getMissionZones().at(missionZone).areas;
				if (!areas.empty())
				{
					targetArea = RNG::generate(0, (int)areas.size() - 1);
				}
			}
			mission->setMissionSiteZoneArea(targetArea);
			mission->start(_engine, _globe);
			base->setGenMissionCount(base->getGenMissionCount() + 1); // increase counter, used to check mission limit
			_save.getAlienMissions().push_back(mission);
		}
	}
	else if (!missionName.empty())
	{
		throw Exception("Alien Base tried to generate undefined mission: " + missionName);
	}
}

/**
 * Takes care of any game logic that has to
 * run every game day, like constructions.
 */
void GeoscapeState::time1Day()
{
	SavedGame *saveGame = getGame()->getSavedGame();
	Mod *mod = getGame()->getMod();
	bool psiStrengthEval = (Options::psiStrengthEval && saveGame->isResearched(mod->getPsiRequirements()));
	for (Base& xcomBase : getRegistry().list<Base>())
	{
		// Handle facility construction
		std::map<const RuleBaseFacility*, int> finishedFacilities;
		for (BaseFacility* facility : xcomBase.getFacilities())
		{
			if (facility->getBuildTime() > 0)
			{
				facility->build();
				if (facility->getBuildTime() == 0)
				{
					finishedFacilities[facility->getRules()] += 1;
				}
			}
		}
		for (const std::pair<const RuleBaseFacility*, int>& pair : finishedFacilities)
		{
			if (pair.second > 1)
			{
				std::ostringstream ssf;
				ssf << tr(pair.first->getType()) << " (x" << pair.second << ")";
				popup(new ProductionCompleteState(&xcomBase, ssf.str(), this, PROGRESS_CONSTRUCTION));
			}
			else
			{
				popup(new ProductionCompleteState(&xcomBase, tr(pair.first->getType()), this, PROGRESS_CONSTRUCTION));
			}
		}

		// Handle science project
		// 1. gather finished research
		std::vector<ResearchProject*> finished;
		for (auto* proj : xcomBase.getResearch())
		{
			if (proj->step())
			{
				finished.push_back(proj);
			}
		}
		// 2. remember available research before adding new finished research
		std::vector<RuleResearch*> before;
		if (!finished.empty())
		{
			saveGame->getAvailableResearchProjects(before, mod, &xcomBase);
		}
		// 3. add finished research, including lookups and getonefrees (up to 4x)
		std::vector<const RuleResearch*> topicsToCheck;
		for (auto* project : finished)
		{
			const RuleResearch *bonus = 0;
			const RuleResearch *research = project->getRules();

			// 3a. remove finished research from the base where it was researched
			xcomBase.removeResearch(project);
			project = nullptr;

			// 3b. handle interrogation
			if (Options::retainCorpses && research->needItem() && research->destroyItem())
			{
				auto* ruleUnit = mod->getUnit(research->getName(), false); // don't use getNeededItem()
				if (ruleUnit)
				{
					auto ruleCorpse = ruleUnit->getArmor()->getCorpseGeoscape();
					if (ruleCorpse && ruleCorpse->isRecoverable() && ruleCorpse->isCorpseRecoverable())
					{
						xcomBase.getStorageItems()->addItem(ruleCorpse);
					}
				}
			}
			// 3c. handle getonefrees (topic+lookup)
			if ((bonus = saveGame->selectGetOneFree(research)))
			{
				saveGame->addFinishedResearch(bonus, mod, &xcomBase);
				if (!bonus->getLookup().empty())
				{
					saveGame->addFinishedResearch(mod->getResearch(bonus->getLookup(), true), mod, &xcomBase);
				}
			}
			// 3d. determine and remember if the ufopedia article should pop up again or not
			// Note: because different topics may lead to the same lookup
			const RuleResearch *newResearch = research;
			std::string name = research->getLookup().empty() ? research->getName() : research->getLookup();
			if (saveGame->isResearched(name, false))
			{
				newResearch = 0;
			}
			// 3e. handle core research (topic+lookup)
			saveGame->addFinishedResearch(research, mod, &xcomBase);
			if (!research->getLookup().empty())
			{
				saveGame->addFinishedResearch(mod->getResearch(research->getLookup(), true), mod, &xcomBase);
			}
			// 3e. handle cutscene
			if (!research->getCutscene().empty())
			{
				popup(new CutsceneState(research->getCutscene()));
				if (saveGame->getEnding() == END_NONE)
				{
					const RuleVideo* videoRule = getGame()->getMod()->getVideo(research->getCutscene(), true);
					if (videoRule->getWinGame()) saveGame->setEnding(END_WIN);
					if (videoRule->getLoseGame()) saveGame->setEnding(END_LOSE);
				}
			}
			if (bonus && !bonus->getCutscene().empty())
			{
				popup(new CutsceneState(bonus->getCutscene()));
				if (saveGame->getEnding() == END_NONE)
				{
					const RuleVideo* videoRule = getGame()->getMod()->getVideo(bonus->getCutscene(), true);
					if (videoRule->getWinGame()) saveGame->setEnding(END_WIN);
					if (videoRule->getLoseGame()) saveGame->setEnding(END_LOSE);
				}
			}
			// 3e. handle research complete popup + ufopedia article popups (topic+bonus)
			popup(new ResearchCompleteState(newResearch, bonus, research, &xcomBase));
			// 3f. reset timer
			timerReset();
			// 3g. warning if weapon is researched before its clip
			if (newResearch)
			{
				RuleItem *item = mod->getItem(newResearch->getName());
				if (item && item->getBattleType() == BT_FIREARM && !item->getPrimaryCompatibleAmmo()->empty())
				{
					RuleManufacture *man = mod->getManufacture(item->getType());
					if (man && !man->getRequirements().empty())
					{
						const auto &req = man->getRequirements();
						const RuleItem *ammo = item->getPrimaryCompatibleAmmo()->front();
						if (std::find_if(req.begin(), req.end(), [&](const RuleResearch* r){ return r->getName() == ammo->getType(); }) != req.end() && !saveGame->isResearched(req, true))
						{
							popup(new ResearchRequiredState(item));
						}
					}
				}
			}
			// 3h. inform about new possible research
			std::vector<RuleResearch *> after;
			saveGame->getAvailableResearchProjects(after, mod, &xcomBase);
			std::vector<RuleResearch *> newPossibleResearch;
			saveGame->getNewlyAvailableResearchProjects(before, after, newPossibleResearch);
			popup(new NewPossibleResearchState(&xcomBase, newPossibleResearch));
			// 3i. inform about new possible manufacture, purchase, craft and facilities
			std::vector<RuleManufacture *> newPossibleManufacture;
			saveGame->getDependableManufacture(newPossibleManufacture, research, mod, &xcomBase);
			if (bonus)
			{
				saveGame->getDependableManufacture(newPossibleManufacture, bonus, mod, &xcomBase);
			}
			if (!newPossibleManufacture.empty())
			{
				Collections::sortVector(newPossibleManufacture);
				Collections::sortVectorMakeUnique(newPossibleManufacture);
				popup(new NewPossibleManufactureState(&xcomBase, newPossibleManufacture));
			}
			std::vector<RuleItem *> newPossiblePurchase;
			getGame()->getSavedGame()->getDependablePurchase(newPossiblePurchase, research, getGame()->getMod());
			if (bonus)
			{
				getGame()->getSavedGame()->getDependablePurchase(newPossiblePurchase, bonus, getGame()->getMod());
			}
			if (!newPossiblePurchase.empty())
			{
				Collections::sortVector(newPossiblePurchase);
				Collections::sortVectorMakeUnique(newPossiblePurchase);
				popup(new NewPossiblePurchaseState(&xcomBase, newPossiblePurchase));
			}
			std::vector<RuleCraft *> newPossibleCraft;
			getGame()->getSavedGame()->getDependableCraft(newPossibleCraft, research, getGame()->getMod());
			if (bonus)
			{
				getGame()->getSavedGame()->getDependableCraft(newPossibleCraft, bonus, getGame()->getMod());
			}
			if (!newPossibleCraft.empty())
			{
				Collections::sortVector(newPossibleCraft);
				Collections::sortVectorMakeUnique(newPossibleCraft);
				popup(new NewPossibleCraftState(&xcomBase, newPossibleCraft));
			}
			std::vector<RuleBaseFacility *> newPossibleFacilities;
			getGame()->getSavedGame()->getDependableFacilities(newPossibleFacilities, research, getGame()->getMod());
			if (bonus)
			{
				getGame()->getSavedGame()->getDependableFacilities(newPossibleFacilities, bonus, getGame()->getMod());
			}
			if (!newPossibleFacilities.empty())
			{
				Collections::sortVector(newPossibleFacilities);
				Collections::sortVectorMakeUnique(newPossibleFacilities);
				popup(new NewPossibleFacilityState(&xcomBase, _globe, newPossibleFacilities));
			}

			topicsToCheck.push_back(research);
			if (bonus)
			{
				topicsToCheck.push_back(bonus);
			}
		}
		if (!topicsToCheck.empty())
		{
			Collections::sortVector(topicsToCheck);
			Collections::sortVectorMakeUnique(topicsToCheck);
			// Side effects:
			// 1. remove obsolete research projects from all bases
			// 2. handle items spawned by research
			// 3. handle events spawned by research
			saveGame->handlePrimaryResearchSideEffects(topicsToCheck, getGame()->getMod(), &xcomBase);
		}

		// Handle soldier wounds and martial training
		BaseSumDailyRecovery recovery = xcomBase.getSumRecoveryPerDay();
		std::vector<Soldier *> trainingFinishedList;
		for (Soldier* soldier : xcomBase.getSoldiers())
		{
			soldier->replenishStats(recovery);

			if (soldier->isInTraining())
			{
				soldier->trainPhys(getGame()->getMod()->getCustomTrainingFactor());
				soldier->calcStatString(getGame()->getMod()->getStatStrings(), psiStrengthEval);
				if (soldier->isFullyTrained())
				{
					soldier->setTraining(false);
					trainingFinishedList.push_back(soldier);
				}
			}
			else
			{
				if (soldier->getReturnToTrainingWhenHealed() && !soldier->isWounded())
				{
					if (!soldier->isFullyTrained() && xcomBase.getFreeTrainingSpace() > 0)
					{
						soldier->setTraining(true);
					}
					// only ever try to return once
					soldier->setReturnToTrainingWhenHealed(false);
				}
			}
		}
		if (!trainingFinishedList.empty())
		{
			popup(new TrainingFinishedState(&xcomBase, trainingFinishedList, false));
		}
		// Handle psionic training
		if (xcomBase.getAvailablePsiLabs() > 0 && Options::anytimePsiTraining)
		{
			std::vector<Soldier*> psiTrainingFinishedList;
			for (Soldier* s : xcomBase.getSoldiers())
			{
				s->trainPsi1Day();
				s->calcStatString(getGame()->getMod()->getStatStrings(), psiStrengthEval);
				if (s->isInPsiTraining() && s->isFullyPsiTrained())
				{
					s->setPsiTraining(false);
					psiTrainingFinishedList.push_back(s);
				}
			}
			if (!psiTrainingFinishedList.empty())
			{
				popup(new TrainingFinishedState(&xcomBase, psiTrainingFinishedList, true));
			}
		}

		// check and remove disabled projects from ongoing research
		std::vector<ResearchProject*> obsolete;
		for (ResearchProject* proj : xcomBase.getResearch())
		{
			if (getGame()->getSavedGame()->isResearchRuleStatusDisabled(proj->getRules()->getName()))
			{
				obsolete.push_back(proj);
			}
		}
		for (auto* proj : obsolete)
		{
			xcomBase.removeResearch(proj);
		}

		// clear the daily dogfight experience cache
		for (Soldier* soldier : xcomBase.getSoldiers())
		{
			soldier->resetDailyDogfightExperienceCache();
		}
	}

	// check and interrupt alien missions if necessary (based on discovered research)
	for (AlienMission* am : saveGame->getAlienMissions())
	{
		auto& researchName = am->getRules().getInterruptResearch();
		if (!researchName.empty())
		{
			auto* research = mod->getResearch(researchName, true);
			if (saveGame->isResearched(research, false)) // ignore debug mode
			{
				am->setInterrupted(true);
			}
		}
	}

	// check and self-destruct alien bases if necessary (based on discovered research)
	auto abIt = saveGame->getAlienBases().begin();
	while (abIt != saveGame->getAlienBases().end())
	{
		AlienBase* ab = (*abIt);
		auto& selfDestructCode = ab->getDeployment()->getBaseSelfDestructCode();
		if (!selfDestructCode.empty())
		{
			auto research = mod->getResearch(selfDestructCode, true);
			if (saveGame->isResearched(research, false)) // ignore debug mode
			{
				saveGame->clearLinksForAlienBase(ab, getGame()->getMod());
				delete ab;
				abIt = saveGame->getAlienBases().erase(abIt);
			}
			else
			{
				++abIt;
			}
		}
		else
		{
			++abIt;
		}
	}

	// handle regional and country points for alien bases
	for (AlienBase* ab : saveGame->getAlienBases())
	{
		AreaSystem::addAlienActivityToCountryAndRegion(ab->getDeployment()->getPoints(), *ab);
	}

	// Handle resupply of alien bases.
	std::for_each(saveGame->getAlienBases().begin(), saveGame->getAlienBases().end(),
			  GenerateSupplyMission(*getGame(), *_globe));

	// Handle alien base detection (by xcom base facilities).
	for (AlienBase* alienBase : getGame()->getSavedGame()->getAlienBases())
	{
		if (alienBase->isDiscovered()) continue;
		for (Base& xcomBase : getRegistry().list<Base>())
		{
			int distance = XcomDistance(xcomBase.getDistance(alienBase));
			for (BaseFacility* facility : xcomBase.getFacilities())
			{
				if (facility->getBuildTime() == 0 && facility->getRules()->getSightRange() > distance)
				{
					int chanceToDetect = facility->getRules()->getSightChance(); // static % defined by the modder
					if (chanceToDetect == 0)
					{
						chanceToDetect = 50 - (distance * 50 / facility->getRules()->getSightRange()); // dynamic 0-50% based on relative distance
					}
					if (RNG::percent(chanceToDetect))
					{
						alienBase->setDiscovered(true);
						popup(new AlienBaseState(alienBase, this));
					}
				}
			}
		}
	}

	// Autosave 3 times a month
	int day = saveGame->getTime()->getDay();
	if (day == 10 || day == 20)
	{
		if (saveGame->isIronman())
		{
			popup(new SaveGameState(OPT_GEOSCAPE, SAVE_IRONMAN, _palette));
		}
		else if (Options::autosave)
		{
			popup(new SaveGameState(OPT_GEOSCAPE, SAVE_AUTO_GEOSCAPE, _palette));
		}
	}
	else if (saveGame->getEnding() != END_NONE && saveGame->isIronman())
	{
		getGame()->pushState(new SaveGameState(OPT_GEOSCAPE, SAVE_IRONMAN, _palette));
	}

	// pay attention to your maintenance player!
	if (getGame()->getSavedGame()->getTime()->isLastDayOfMonth())
	{
		int month = getGame()->getSavedGame()->getMonthsPassed();
		int currentScore = getGame()->getSavedGame()->getCurrentScore(month + 1);
		int performanceBonus = std::max(0, currentScore * mod->getPerformanceBonusFactor());

		int64_t funds = getGame()->getSavedGame()->getFunds();
		int64_t income = CountrySystem::getCountriesMonthlyFundingTotal() + performanceBonus;
		int64_t maintenance = BaseSystem::getBasesMaintenanceCost();
		int64_t projection = funds + income - maintenance;
		if (projection < 0)
		{
			projection = std::abs(projection);
			projection = ((projection / 100000) + 1) * 100000; // round up to 100k
			std::string msg = tr("STR_ECONOMY_WARNING")
				.arg(Unicode::formatFunding(funds))
				.arg(Unicode::formatFunding(income))
				.arg(Unicode::formatFunding(maintenance))
				.arg(Unicode::formatFunding(projection));
			popup(new CraftErrorState(this, msg, false));
		}
	}
}

/**
 * Takes care of any game logic that has to
 * run every game month, like funding.
 */
void GeoscapeState::time1Month()
{
	getGame()->getSavedGame()->addMonth();

	// Determine alien mission for this month.
	determineAlienMissions();

	// Handle Psi-Training and initiate a new retaliation mission, if applicable
	if (!Options::anytimePsiTraining)
	{
		bool psiStrengthEval = (Options::psiStrengthEval && getGame()->getSavedGame()->isResearched(getGame()->getMod()->getPsiRequirements()));

		auto availablePsiLabs = [](const Base& base) { return base.getAvailablePsiLabs() > 0; };
		for (Base& xcomBase : getRegistry().list<Base>(availablePsiLabs))
		{
			for (Soldier* soldier : xcomBase.getSoldiers())
			{
				if (soldier->isInPsiTraining())
				{
					soldier->trainPsi();
					soldier->calcStatString(getGame()->getMod()->getStatStrings(), psiStrengthEval);
				}
			}
		}
	}

	// Handle funding
	timerReset();
	getGame()->getSavedGame()->monthlyFunding();
	popup(new MonthlyReportState(_globe));

	// Handle Xcom Operatives discovering bases
	if (!getGame()->getSavedGame()->getAlienBases().empty() && RNG::percent(20))
	{
		for (AlienBase* ab : getGame()->getSavedGame()->getAlienBases())
		{
			if (!ab->isDiscovered())
			{
				ab->setDiscovered(true);
				popup(new AlienBaseState(ab, this));
				break;
			}
		}
	}
}

/**
 * Slows down the timer back to minimum speed,
 * for when important events occur.
 */
void GeoscapeState::timerReset()
{
	SDL_Event ev{};
	ev.button.button = SDL_BUTTON_LEFT;
	Action act(&ev, getGame()->getScreen()->getXScale(), getGame()->getScreen()->getYScale(), getGame()->getScreen()->getCursorTopBlackBand(), getGame()->getScreen()->getCursorLeftBlackBand());
	_btn5Secs->mousePress(&act, this);
}

/**
 * Adds a new popup window to the queue
 * (this prevents popups from overlapping)
 * and pauses the game timer respectively.
 * @param state Pointer to popup state.
 */
void GeoscapeState::popup(State *state)
{
	_pause = true;
	_popups.push_back(state);
}

/**
 * Processes any left-clicks on globe markers,
 * or right-clicks to scroll the globe.
 * @param action Pointer to an action.
 */
void GeoscapeState::globeClick(Action *action)
{
	int mouseX = (int)floor(action->getAbsoluteXMouse()), mouseY = (int)floor(action->getAbsoluteYMouse());

	// Clicking markers on the globe
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		std::vector<Target*> v = _globe->getTargets(mouseX, mouseY, false, 0);
		if (!v.empty())
		{
			// Pass empty vector
			std::vector<Craft*> crafts;
			getGame()->pushState(new MultipleTargetsState(v, crafts, this, true));
		}
	}

	if (getGame()->getSavedGame()->getDebugMode())
	{
		double lon, lat;
		int texture, shade;
		_globe->cartToPolar(mouseX, mouseY, &lon, &lat);
		double lonDeg = lon / M_PI * 180, latDeg = lat / M_PI * 180;
		_globe->getPolygonTextureAndShade(lon, lat, &texture, &shade);
		std::ostringstream ss;
		ss << "rad: " << lon << ", " << lat << std::endl;
		ss << "deg: " << lonDeg << ", " << latDeg << std::endl;
		ss << "texture: " << texture << ", shade: " << shade << std::endl;

		_txtDebug->setText(ss.str());
	}
}

/**
 * Opens the Intercept window.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnInterceptClick(Action *)
{
	if (buttonsDisabled())
	{
		return;
	}
	getGame()->pushState(new InterceptState(_globe, false));
}

/**
* Opens the UFO Tracker window.
* @param action Pointer to an action.
*/
void GeoscapeState::btnUfoTrackerClick(Action *)
{
	getGame()->pushState(new UfoTrackerState(this, _globe));
}

/**
* Opens the TechTreeViewer window.
* @param action Pointer to an action.
*/
void GeoscapeState::btnTechTreeViewerClick(Action *)
{
	getGame()->pushState(new TechTreeViewerState());
}

/**
 * Opens the jukebox.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnSelectMusicTrackClick(Action *)
{
	getGame()->pushState(new SelectMusicTrackState(SMT_GEOSCAPE));
}

/**
 * Opens the Current Global Production.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnGlobalProductionClick(Action *)
{
	getGame()->pushState(new GlobalManufactureState(false));
}

/**
 * Opens the Current Global Research.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnGlobalResearchClick(Action *)
{
	getGame()->pushState(new GlobalResearchState(false));
}

/**
 * Opens the Global Alien Containment overview.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnGlobalAlienContainmentClick(Action *)
{
	getGame()->pushState(new GlobalAlienContainmentState(false));
}

/**
 * Opens the Dogfight Experience screen.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnDogfightExperienceClick(Action *)
{
	getGame()->pushState(new DogfightExperienceState());
}

/**
 * Toggles debug mode.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnDebugClick(Action *)
{
	getGame()->getSavedGame()->setDebugMode();
	if (getGame()->getSavedGame()->getDebugMode())
	{
		_txtDebug->setText("DEBUG MODE");
	}
	else
	{
		_txtDebug->setText("");
	}
	_cbxRegion->setVisible(getGame()->getSavedGame()->getDebugMode() && getGame()->getSavedGame()->debugType >= 1);
	_cbxZone->setVisible(getGame()->getSavedGame()->getDebugMode() && getGame()->getSavedGame()->debugType == 2);
	_cbxArea->setVisible(getGame()->getSavedGame()->getDebugMode() && getGame()->getSavedGame()->debugType == 2);
	_cbxCountry->setVisible(getGame()->getSavedGame()->getDebugMode() && getGame()->getSavedGame()->debugType == 0);
}

/**
 * Goes to the Basescape screen.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnBasesClick(Action *)
{
	if (buttonsDisabled()) { return; }

	timerReset();
	getGame()->pushState(new BasescapeState(getRegistry().getService<BasescapeSystem>(), _globe));
}

/**
 * Goes to the Graphs screen.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnGraphsClick(Action *)
{
	if (buttonsDisabled()) { return; }
	getGame()->pushState(new GraphsState);
}

/**
 * Goes to the Ufopaedia window.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnUfopaediaClick(Action *)
{
	if (buttonsDisabled()) { return; }
	Ufopaedia::open(getGame());
}

/**
 * Opens the Options window.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnOptionsClick(Action *)
{
	if (buttonsDisabled()) { return; }
	getGame()->pushState(new PauseState(OPT_GEOSCAPE));
}

/**
 * Goes to the Funding screen.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnFundingClick(Action *)
{
	if (buttonsDisabled()) { return; }
	if (Options::oxceLinks)
	{
		getGame()->pushState(new ExtendedGeoscapeLinksState(this));
	}
	else
	{
		getGame()->pushState(new FundingState);
	}
}

/**
 * Starts rotating the globe to the left.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnRotateLeftPress(Action *)
{
	_globe->rotateLeft();
}

/**
 * Stops rotating the globe to the left.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnRotateLeftRelease(Action *)
{
	_globe->rotateStopLon();
}

/**
 * Starts rotating the globe to the right.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnRotateRightPress(Action *)
{
	_globe->rotateRight();
}

/**
 * Stops rotating the globe to the right.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnRotateRightRelease(Action *)
{
	_globe->rotateStopLon();
}

/**
 * Starts rotating the globe upwards.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnRotateUpPress(Action *)
{
	_globe->rotateUp();
}

/**
 * Stops rotating the globe upwards.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnRotateUpRelease(Action *)
{
	_globe->rotateStopLat();
}

/**
 * Starts rotating the globe downwards.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnRotateDownPress(Action *)
{
	_globe->rotateDown();
}

/**
 * Stops rotating the globe downwards.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnRotateDownRelease(Action *)
{
	_globe->rotateStopLat();
}

/**
 * Zooms into the globe.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnZoomInLeftClick(Action *)
{
	_globe->zoomIn();
}

/**
 * Zooms the globe maximum.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnZoomInRightClick(Action *)
{
	_globe->zoomMax();
}

/**
 * Zooms out of the globe.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnZoomOutLeftClick(Action *)
{
	_globe->zoomOut();
}

/**
 * Zooms the globe minimum.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnZoomOutRightClick(Action *)
{
	_globe->zoomMin();
}

/**
 * Zoom in effect for dogfights.
 */
void GeoscapeState::zoomInEffect()
{
	if (_globe->zoomDogfightIn())
	{
		_zoomInEffectDone = true;
		_zoomInEffectTimer->stop();
	}
}

/**
 * Zoom out effect for dogfights.
 */
void GeoscapeState::zoomOutEffect()
{
	if (_globe->zoomDogfightOut())
	{
		_zoomOutEffectDone = true;
		_zoomOutEffectTimer->stop();
		init();
	}
}

/**
 * Dogfight logic. Moved here to have the code clean.
 */
void GeoscapeState::handleDogfights()
{
	// Handle dogfights logic.
	_minimizedDogfights = 0;

	for (auto* dfs : _dogfights)
	{
		dfs->getUfo()->setInterceptionProcessed(false);
	}

	auto dfsIt = _dogfights.begin();
	while (dfsIt != _dogfights.end())
	{
		DogfightState* dfs = (*dfsIt);
		if (dfs->isMinimized())
		{
			if (dfs->getWaitForPoly() && _globe->insideLand(dfs->getUfo()->getLongitude(), dfs->getUfo()->getLatitude()))
			{
				dfs->setMinimized(false);
				dfs->setWaitForPoly(false);
			}
			else if (dfs->getWaitForAltitude() && dfs->getUfo()->getAltitudeInt() <= dfs->getCraft()->getRules()->getMaxAltitude())
			{
				dfs->setMinimized(false);
				dfs->setWaitForAltitude(false);
			}
			else
			{
				_minimizedDogfights++;
			}
		}
		else
		{
			_globe->rotateStop();
		}
		dfs->update();
		if (dfs->dogfightEnded())
		{
			if (dfs->isMinimized())
			{
				_minimizedDogfights--;
			}
			delete dfs;
			dfsIt = _dogfights.erase(dfsIt);
		}
		else
		{
			++dfsIt;
		}
	}
	if (_dogfights.empty())
	{
		_dogfightTimer->stop();
		_zoomOutEffectTimer->start();
	}
}

/**
 * Goes through all active dogfight instances and tries to perform the same action.
 * @param button Action to perform.
 */
void GeoscapeState::handleDogfightMultiAction(int button)
{
	for (auto* dfs : _dogfights)
	{
		if (dfs->isMinimized())
			continue;

		SDL_Event ev;
		ev.type = SDL_MOUSEBUTTONDOWN;
		ev.button.button = SDL_BUTTON_LEFT;
		Action a = Action(&ev, 0.0, 0.0, 0, 0);

		switch (button)
		{
		case 0:
			dfs->btnStandoffSimulateLeftPress(&a);
			break;
		case 1:
			dfs->btnCautiousSimulateLeftPress(&a);
			break;
		case 2:
			dfs->btnStandardSimulateLeftPress(&a);
			break;
		case 3:
			dfs->btnAggressiveSimulateLeftPress(&a);
			break;
		case 4:
			dfs->btnDisengageSimulateLeftPress(&a);
			break;
		default:
			break;
		}
	}
}

/**
 * Goes through all dogfight instances and tries to award pilot experience.
 * This is called each time any UFO takes any damage in dogfight... very ugly, but I couldn't find a better place for it.
 *
 * History lesson:
 * - this was in the DogfightState destructor before, but could lead to CTD when people reloaded a saved game while dogfights were still active
 * - both OXCE and vanilla OXC still access already deleted objects (_craft and _ufo) in the destructor... that doesn't seem to trigger CTD that often though
 */
void GeoscapeState::handleDogfightExperience()
{
	for (auto* dfs : _dogfights)
	{
		dfs->awardExperienceToPilots();
	}
	for (auto* dfs : _dogfightsToBeStarted)
	{
		dfs->awardExperienceToPilots();
	}
}

/**
 * Gets the number of minimized dogfights.
 * @return Number of minimized dogfights.
 */
int GeoscapeState::minimizedDogfightsCount()
{
	int minimizedDogfights = 0;
	for (auto* dfs : _dogfights)
	{
		if (dfs->isMinimized())
		{
			++minimizedDogfights;
		}
	}
	return minimizedDogfights;
}

/**
 * Starts a new dogfight.
 */
void GeoscapeState::startDogfight()
{
	if (_globe->getZoom() < 3)
	{
		if (!_zoomInEffectTimer->isRunning())
		{
			_globe->saveZoomDogfight();
			_globe->rotateStop();
			_zoomInEffectTimer->start();
		}
	}
	else
	{
		_dogfightStartTimer->stop();
		_zoomInEffectTimer->stop();
		_dogfightTimer->start();
		timerReset();
		while (!_dogfightsToBeStarted.empty())
		{
			_dogfights.push_back(_dogfightsToBeStarted.back());
			_dogfightsToBeStarted.pop_back();
			_dogfights.back()->setInterceptionNumber(getFirstFreeDogfightSlot());
			_dogfights.back()->setInterceptionsCount(_dogfights.size() + _dogfightsToBeStarted.size());
		}
		// Set correct number of interceptions for every dogfight.
		for (auto* dfs : _dogfights)
		{
			dfs->setInterceptionsCount(_dogfights.size());
		}
	}
}

/**
 * Returns the first free dogfight slot.
 * @return free slot
 */
int GeoscapeState::getFirstFreeDogfightSlot()
{
	int slotNo = 1;
	for (auto* dfs : _dogfights)
	{
		if (dfs->getInterceptionNumber() == slotNo)
		{
			++slotNo;
		}
	}
	return slotNo;
}

/**
 * Handle base defense
 * @param base Base to defend.
 * @param ufo Ufo attacking base.
 */
void GeoscapeState::handleBaseDefense(Base *base, Ufo *ufo)
{
	// Get the shade and texture for the globe at the location of the base, using the ufo position
	int texture, shade;
	double baseLon = ufo->getLongitude();
	double baseLat = ufo->getLatitude();
	_globe->getPolygonTextureAndShade(baseLon, baseLat, &texture, &shade);

	int ufoDamagePercentage = 0;
	if (getGame()->getMod()->getLessAliensDuringBaseDefense())
	{
		ufoDamagePercentage = ufo->getDamagePercentage();
	}

	// Whatever happens in the base defense, the UFO has finished its duty
	ufo->setStatus(Ufo::DESTROYED);

	// instant retaliation mission only spawns one UFO and then ends
	if (ufo->getMission()->getRules().getObjective() == OBJECTIVE_INSTANT_RETALIATION)
	{
		ufo->getMission()->setInterrupted(true);
	}

	if (ufo->getRules()->getMissilePower() != 0)
	{
		if (ufo->getRules()->getMissilePower() < 0)
		{
			// It's a nuclear warhead... Skynet knows no mercy
			popup(new BaseDestroyedState(base, ufo, true, false));
		}
		else
		{
			// This is an overkill, since we may not lose any hangar/craft, but doing it properly requires tons of changes
			getGame()->getSavedGame()->stopHuntingXcomCrafts(base);

			// This can either damage facilities (=replace them with some other facilities); or also destroy facilities
			base->damageFacilities(ufo);

			// don't forget to reset pre-cached stuff
			base->cleanupDefenses(true);

			// let the player know that some facilities were destroyed, but the base survived
			popup(new BaseDestroyedState(base, ufo, true, true));
		}
	}
	else if (base->getAvailableSoldiers(true, true) > 0 || !base->getVehicles().empty())
	{
		SavedBattleGame *bgame = new SavedBattleGame(getGame()->getMod(), getGame()->getLanguage());
		getGame()->getSavedGame()->setBattleGame(bgame);
		bgame->setMissionType("STR_BASE_DEFENSE");
		BattlescapeGenerator bgen = BattlescapeGenerator(getGame());
		bgen.setBase(base);
		bgen.setAlienCustomDeploy(getGame()->getMod()->getDeployment(ufo->getCraftStats().missionCustomDeploy));
		bgen.setAlienRace(ufo->getAlienRace());
		bgen.setWorldShade(shade);
		auto globeTexture = getGame()->getMod()->getGlobe()->getTexture(texture);
		bgen.setWorldTexture(globeTexture, globeTexture);
		bgen.setUfoDamagePercentage(ufoDamagePercentage);
		bgen.run();
		_pause = true;
		getGame()->pushState(new BriefingState(0, base));
	}
	else
	{
		// Please garrison your bases in future
		popup(new BaseDestroyedState(base, ufo, false, false));
	}
}

/**
 * Determine the alien missions to start this month.
 */
void GeoscapeState::determineAlienMissions()
{
	SavedGame *save = getGame()->getSavedGame();
	AlienStrategy &strategy = save->getAlienStrategy();
	Mod *mod = getGame()->getMod();
	int month = getGame()->getSavedGame()->getMonthsPassed();
	int currentScore = save->getCurrentScore(month); // _monthsPassed was already increased by 1
	int performanceBonus = std::max(0, currentScore * mod->getPerformanceBonusFactor());
	int64_t currentFunds = save->getFunds();
	int64_t montlyFunding = CountrySystem::getCountriesMonthlyFundingTotal();
	int64_t baseMaintenance = BaseSystem::getBasesMaintenanceCost();
	currentFunds += montlyFunding + performanceBonus - baseMaintenance; // peek into the next month
	std::vector<RuleMissionScript*> availableMissions;
	std::map<int, bool> conditions;

	std::set<std::string> xcomBaseRegions;
	std::set<std::string> xcomBaseCountries;
	for (Base& xcomBase : getRegistry().list<Base>())
	{
		if (Region* region = AreaSystem::locateValue<Region>(xcomBase))
		{
			xcomBaseRegions.insert(region->getRules()->getType());
		}
		if (Country* country = AreaSystem::locateValue<Country>(xcomBase))
		{
			xcomBaseCountries.insert(country->getRules()->getType());
		}
	}

	// sorry to interrupt, but before we start determining the actual monthly missions, let's determine and/or adjust our overall game plan
	{
		std::vector<RuleArcScript*> relevantArcScripts;

		// first we need to build a list of "valid" commands
		for (const std::string& scriptName : mod->getArcScriptList())
		{
			RuleArcScript* arcScript = mod->getArcScript(scriptName);

			// level one condition check: make sure we're within our time constraints
			if (arcScript->getFirstMonth() <= month &&
				(arcScript->getLastMonth() >= month || arcScript->getLastMonth() == -1) &&
				// and make sure we satisfy the difficulty restrictions
				(month < 1 || arcScript->getMinScore() <= currentScore) &&
				(month < 1 || arcScript->getMaxScore() >= currentScore) &&
				(month < 1 || arcScript->getMinFunds() <= currentFunds) &&
				(month < 1 || arcScript->getMaxFunds() >= currentFunds) &&
				arcScript->getMinDifficulty() <= save->getDifficulty() &&
				arcScript->getMaxDifficulty() >= save->getDifficulty())
			{
				// level two condition check: make sure we meet any research requirements, if any.
				bool triggerHappy = true;
				for (auto& trigger : arcScript->getResearchTriggers())
				{
					triggerHappy = (save->isResearched(trigger.first) == trigger.second);
					if (!triggerHappy)
						break;
				}
				if (triggerHappy)
				{
					// check counters
					if (arcScript->getCounterMin() > 0)
					{
						if (!arcScript->getMissionVarName().empty() && arcScript->getCounterMin() > strategy.getMissionsRun(arcScript->getMissionVarName()))
						{
							triggerHappy = false;
						}
						if (!arcScript->getMissionMarkerName().empty() && arcScript->getCounterMin() > save->getLastId(arcScript->getMissionMarkerName()))
						{
							triggerHappy = false;
						}
					}
					if (triggerHappy && arcScript->getCounterMax() != -1)
					{
						if (!arcScript->getMissionVarName().empty() && arcScript->getCounterMax() < strategy.getMissionsRun(arcScript->getMissionVarName()))
						{
							triggerHappy = false;
						}
						if (!arcScript->getMissionMarkerName().empty() && arcScript->getCounterMax() < save->getLastId(arcScript->getMissionMarkerName()))
						{
							triggerHappy = false;
						}
					}
				}
				if (triggerHappy)
				{
					// item requirements
					for (auto &triggerItem : arcScript->getItemTriggers())
					{
						triggerHappy = BaseSystem::isItemInBaseStores(triggerItem.first) == triggerItem.second;
						if (!triggerHappy)
							break;
					}
				}
				if (triggerHappy)
				{
					// facility requirements
					for (auto &triggerFacility : arcScript->getFacilityTriggers())
					{
						triggerHappy = (BaseSystem::isFacilityBuilt(triggerFacility.first) == triggerFacility.second);
						if (!triggerHappy)
							break;
					}
				}
				if (triggerHappy)
				{
					// xcom base requirements
					for (auto& triggerXcomBase : arcScript->getXcomBaseInRegionTriggers())
					{
						bool found = (xcomBaseRegions.find(triggerXcomBase.first) != xcomBaseRegions.end());
						triggerHappy = (found == triggerXcomBase.second);
						if (!triggerHappy)
							break;
					}
				}
				if (triggerHappy)
				{
					// xcom base requirements by country
					for (auto& triggerXcomBase2 : arcScript->getXcomBaseInCountryTriggers())
					{
						bool found = (xcomBaseCountries.find(triggerXcomBase2.first) != xcomBaseCountries.end());
						triggerHappy = (found == triggerXcomBase2.second);
						if (!triggerHappy)
							break;
					}
				}
				// level three condition check: does random chance favour this command's execution?
				if (triggerHappy && RNG::percent(arcScript->getExecutionOdds()))
				{
					relevantArcScripts.push_back(arcScript);
				}
			}
		}

		// start processing command array
		for (auto* arcCommand : relevantArcScripts)
		{
			// to remember stuff we can still enable
			std::vector<std::string> disabledSeqArcs;
			WeightedOptions disabledRngArcs;

			int arcsEnabled = 0;
			// level four condition check: check maxArcs (duplicates count, arcs enabled by other commands or in any other way count too!)
			{
				for (auto& seqArc : arcCommand->getSequentialArcs())
				{
					if (save->isResearched(seqArc))
						++arcsEnabled;
					else
						disabledSeqArcs.push_back(seqArc);
				}
				WeightedOptions tmp = arcCommand->getRandomArcs(); // copy for the iterator, because of getNames()
				disabledRngArcs = tmp; // copy for us to modify
				for (auto& rngArc : tmp.getNames())
				{
					if (save->isResearched(rngArc))
					{
						++arcsEnabled;
						disabledRngArcs.set(rngArc, 0); // delete
					}
				}
			}
			
			Base* hq = getRegistry().front<Base>().try_get<Base>();
			bool canAddOneMore = arcCommand->getMaxArcs() == -1 || arcCommand->getMaxArcs() > arcsEnabled;
			if (canAddOneMore && !disabledSeqArcs.empty())
			{
				auto ruleResearchSeq = mod->getResearch(disabledSeqArcs.front(), true); // take first
				save->addFinishedResearch(ruleResearchSeq, mod, hq, true);
				++arcsEnabled;
				if (ruleResearchSeq)
				{
					if (ruleResearchSeq->getLookup().empty())
					{
						Ufopaedia::openArticle(getGame(), ruleResearchSeq->getName());
					}
					else
					{
						save->addFinishedResearch(mod->getResearch(ruleResearchSeq->getLookup(), true), mod, hq, true);
						Ufopaedia::openArticle(getGame(), ruleResearchSeq->getLookup());
					}
				}
			}
			canAddOneMore = arcCommand->getMaxArcs() == -1 || arcCommand->getMaxArcs() > arcsEnabled;
			if (canAddOneMore && !disabledRngArcs.empty())
			{
				auto ruleResearchRng = mod->getResearch(disabledRngArcs.choose(), true); // take random
				save->addFinishedResearch(ruleResearchRng, mod, hq, true);
				++arcsEnabled; // for good measure :)
				if (ruleResearchRng)
				{
					if (ruleResearchRng->getLookup().empty())
					{
						Ufopaedia::openArticle(getGame(), ruleResearchRng->getName());
					}
					else
					{
						save->addFinishedResearch(mod->getResearch(ruleResearchRng->getLookup(), true), mod, hq, true);
						Ufopaedia::openArticle(getGame(), ruleResearchRng->getLookup());
					}
				}
			}
		}
	}

	// well, here it is, ladies and gents, the nuts and bolts behind the geoscape mission scheduling.

	// first we need to build a list of "valid" commands
	for (const std::string& missionScriptName : mod->getMissionScriptList())
	{
		RuleMissionScript *command = mod->getMissionScript(missionScriptName);

			// level one condition check: make sure we're within our time constraints
		if (command->getFirstMonth() <= month &&
			(command->getLastMonth() >= month || command->getLastMonth() == -1) &&
			// make sure we haven't hit our run limit, if we have one
			(command->getMaxRuns() == -1 ||	command->getMaxRuns() > strategy.getMissionsRun(command->getVarName())) &&
			// and make sure we satisfy the difficulty restrictions
			(month < 1 || command->getMinScore() <= currentScore) &&
			(month < 1 || command->getMaxScore() >= currentScore) &&
			(month < 1 || command->getMinFunds() <= currentFunds) &&
			(month < 1 || command->getMaxFunds() >= currentFunds) &&
			command->getMinDifficulty() <= save->getDifficulty() &&
			command->getMaxDifficulty() >= save->getDifficulty())
		{
			// level two condition check: make sure we meet any research requirements, if any.
			bool triggerHappy = true;
			for (auto& triggerResearch : command->getResearchTriggers())
			{
				triggerHappy = (save->isResearched(triggerResearch.first) == triggerResearch.second);
				if (!triggerHappy)
					break;
			}
			if (triggerHappy)
			{
				// check counters
				if (command->getCounterMin() > 0)
				{
					if (!command->getMissionVarName().empty() && command->getCounterMin() > strategy.getMissionsRun(command->getMissionVarName()))
					{
						triggerHappy = false;
					}
					if (!command->getMissionMarkerName().empty() && command->getCounterMin() > save->getLastId(command->getMissionMarkerName()))
					{
						triggerHappy = false;
					}
				}
				if (triggerHappy && command->getCounterMax() != -1)
				{
					if (!command->getMissionVarName().empty() && command->getCounterMax() < strategy.getMissionsRun(command->getMissionVarName()))
					{
						triggerHappy = false;
					}
					if (!command->getMissionMarkerName().empty() && command->getCounterMax() < save->getLastId(command->getMissionMarkerName()))
					{
						triggerHappy = false;
					}
				}
			}
			if (triggerHappy)
			{
				// item requirements
				for (auto &triggerItem : command->getItemTriggers())
				{
					triggerHappy = BaseSystem::isItemInBaseStores(triggerItem.first) == triggerItem.second;
					if (!triggerHappy)
						break;
				}
			}
			if (triggerHappy)
			{
				// facility requirements
				for (auto &triggerFacility : command->getFacilityTriggers())
				{
					triggerHappy = BaseSystem::isFacilityBuilt(triggerFacility.first) == triggerFacility.second;
					if (!triggerHappy)
						break;
				}
			}
			if (triggerHappy)
			{
				// xcom base requirements
				for (auto& triggerXcomBase : command->getXcomBaseInRegionTriggers())
				{
					bool found = (xcomBaseRegions.find(triggerXcomBase.first) != xcomBaseRegions.end());
					triggerHappy = (found == triggerXcomBase.second);
					if (!triggerHappy)
						break;
				}
			}
			if (triggerHappy)
			{
				// xcom base requirements by country
				for (auto& triggerXcomBase2 : command->getXcomBaseInCountryTriggers())
				{
					bool found = (xcomBaseCountries.find(triggerXcomBase2.first) != xcomBaseCountries.end());
					triggerHappy = (found == triggerXcomBase2.second);
					if (!triggerHappy)
						break;
				}
			}
			// levels one and two passed: insert this command into the array.
			if (triggerHappy)
			{
				availableMissions.push_back(command);
			}
		}
	}

	// start processing command array.
	for (auto* command : availableMissions)
	{
		bool process = true;
		bool success = false;
		// level three condition check: make sure our conditionals are met, if any. this list is dynamic, and must be checked here.
		for (int condition : command->getConditionals())
		{
			if (!process)
			{
				break; // loop finished
			}
			auto found = conditions.find(std::abs(condition));
			// just an FYI: if you add a 0 to your conditionals, this flag will never resolve to true, and your command will never run.
			process = (found == conditions.end() || (found->second == true && condition > 0) || (found->second == false && condition < 0));
		}
		if (command->getLabel() > 0 && conditions.find(command->getLabel()) != conditions.end())
		{
			std::ostringstream ss;
			ss << "Mission generator encountered an error: multiple commands: " << command->getType() << " and ";
			for (auto* command2 : availableMissions)
			{
				if (command->getLabel() == command2->getLabel() && command2 != command)
				{
					ss << command2->getType() << ", ";
				}
			}
			ss  << "are sharing the same label: " << command->getLabel();
			throw Exception(ss.str());
		}
		// level four condition check: does random chance favour this command's execution?
		if (process)
		{
			bool rngret = RNG::percent(command->getExecutionOdds());
			if (Options::verboseLogging && Options::oxceGeoscapeDebugLogMaxEntries > 0)
			{
				std::ostringstream ss;
				ss << "month: " << month;
				ss << " script: " << command->getType();
				ss << " odds: " << command->getExecutionOdds();
				ss << " rng: " << rngret;
				save->getGeoscapeDebugLog().push_back(ss.str());
			}
			if (rngret)
			{
				// good news, little command pointer! you're FDA approved! off to the main processing facility with you!
				success = processCommand(command);
			}

		}
		if (command->getLabel() > 0)
		{
			// tsk, tsk. you really should be careful with these unique labels, they're supposed to be unique.
			if (conditions.find(command->getLabel()) != conditions.end())
			{
				throw Exception("Error in mission scripts: " + command->getType() + ". Two or more commands sharing the same label. That's bad, Mmmkay?");
			}
			// keep track of what happened to this command, so others may reference it.
			conditions[command->getLabel()] = success;
		}
	}

	// after the mission scripts, it's time for the event scripts
	{
		std::vector<RuleEventScript *> relevantEventScripts;

		// first we need to build a list of "valid" commands
		for (const std::string& scriptName : mod->getEventScriptList())
		{
			RuleEventScript *eventScript = mod->getEventScript(scriptName);

			// level one condition check: make sure we're within our time constraints
			if (eventScript->getFirstMonth() <= month &&
				(eventScript->getLastMonth() >= month || eventScript->getLastMonth() == -1) &&
				// and make sure we satisfy the difficulty restrictions
				(month < 1 || eventScript->getMinScore() <= currentScore) &&
				(month < 1 || eventScript->getMaxScore() >= currentScore) &&
				(month < 1 || eventScript->getMinFunds() <= currentFunds) &&
				(month < 1 || eventScript->getMaxFunds() >= currentFunds) &&
				eventScript->getMinDifficulty() <= save->getDifficulty() &&
				eventScript->getMaxDifficulty() >= save->getDifficulty())
			{
				// level two condition check: make sure we meet any research requirements, if any.
				bool triggerHappy = true;
				for (auto& trigger : eventScript->getResearchTriggers())
				{
					triggerHappy = (save->isResearched(trigger.first) == trigger.second);
					if (!triggerHappy)
						break;
				}
				if (triggerHappy)
				{
					// check counters
					if (eventScript->getCounterMin() > 0)
					{
						if (!eventScript->getMissionVarName().empty() && eventScript->getCounterMin() > strategy.getMissionsRun(eventScript->getMissionVarName()))
						{
							triggerHappy = false;
						}
						if (!eventScript->getMissionMarkerName().empty() && eventScript->getCounterMin() > save->getLastId(eventScript->getMissionMarkerName()))
						{
							triggerHappy = false;
						}
					}
					if (triggerHappy && eventScript->getCounterMax() != -1)
					{
						if (!eventScript->getMissionVarName().empty() && eventScript->getCounterMax() < strategy.getMissionsRun(eventScript->getMissionVarName()))
						{
							triggerHappy = false;
						}
						if (!eventScript->getMissionMarkerName().empty() && eventScript->getCounterMax() < save->getLastId(eventScript->getMissionMarkerName()))
						{
							triggerHappy = false;
						}
					}
				}
				if (triggerHappy)
				{
					// item requirements
					for (auto &triggerItem : eventScript->getItemTriggers())
					{
						triggerHappy = BaseSystem::isItemInBaseStores(triggerItem.first) == triggerItem.second;
						if (!triggerHappy)
							break;
					}
				}
				if (triggerHappy)
				{
					// facility requirements
					for (auto &triggerFacility : eventScript->getFacilityTriggers())
					{
						triggerHappy = BaseSystem::isFacilityBuilt(triggerFacility.first) == triggerFacility.second;
						if (!triggerHappy)
							break;
					}
				}
				if (triggerHappy)
				{
					// soldier type requirements
					for (auto& triggerSoldierType : eventScript->getSoldierTypeTriggers())
					{
						triggerHappy = BaseSystem::isSoldierTypeHired(triggerSoldierType.first) == triggerSoldierType.second;
						if (!triggerHappy)
							break;
					}
				}
				if (triggerHappy)
				{
					// xcom base requirements
					for (auto& triggerXcomBase : eventScript->getXcomBaseInRegionTriggers())
					{
						bool found = (xcomBaseRegions.find(triggerXcomBase.first) != xcomBaseRegions.end());
						triggerHappy = (found == triggerXcomBase.second);
						if (!triggerHappy)
							break;
					}
				}
				if (triggerHappy)
				{
					// xcom base requirements by country
					for (auto& triggerXcomBase2 : eventScript->getXcomBaseInCountryTriggers())
					{
						bool found = (xcomBaseCountries.find(triggerXcomBase2.first) != xcomBaseCountries.end());
						triggerHappy = (found == triggerXcomBase2.second);
						if (!triggerHappy)
							break;
					}
				}
				// level three condition check: does random chance favour this command's execution?
				if (triggerHappy && RNG::percent(eventScript->getExecutionOdds()))
				{
					relevantEventScripts.push_back(eventScript);
				}
			}
		}

		// now, let's process the relevant event scripts
		for (auto& eventCommand : relevantEventScripts)
		{
			std::vector<const RuleEvent*> toBeGenerated;

			// 1. sequentially generated one-time events (cannot repeat)
			{
				std::vector<std::string> possibleSeqEvents;
				for (auto& seqEvent : eventCommand->getOneTimeSequentialEvents())
				{
					if (!save->wasEventGenerated(seqEvent))
						possibleSeqEvents.push_back(seqEvent); // insert
				}
				if (!possibleSeqEvents.empty())
				{
					auto eventRules = mod->getEvent(possibleSeqEvents.front(), true); // take first
					toBeGenerated.push_back(eventRules);
				}
			}

			// 2. randomly generated one-time events (cannot repeat)
			{
				WeightedOptions possibleRngEvents;
				WeightedOptions tmp = eventCommand->getOneTimeRandomEvents(); // copy for the iterator, because of getNames()
				possibleRngEvents = tmp; // copy for us to modify
				for (const auto& rngEvent : tmp.getNames())
				{
					if (save->wasEventGenerated(rngEvent))
						possibleRngEvents.set(rngEvent, 0); // delete
				}
				if (!possibleRngEvents.empty())
				{
					auto eventRules = mod->getEvent(possibleRngEvents.choose(), true); // take random
					toBeGenerated.push_back(eventRules);
				}
			}

			// 3. randomly generated repeatable events
			{
				auto eventRules = mod->getEvent(eventCommand->generate(save->getMonthsPassed()), false);
				if (eventRules)
				{
					toBeGenerated.push_back(eventRules);
				}
			}

			// 4. generate
			for (auto eventRules : toBeGenerated)
			{
				save->spawnEvent(eventRules);
			}
		}
	}

	// Alien base upgrades happen only AFTER the first game month
	if (month > 0)
	{
		for (AlienBase* alienBase : save->getAlienBases())
		{
			auto baseAgeInMonths = month - alienBase->getStartMonth();
			auto upgradeId = alienBase->getDeployment()->generateAlienBaseUpgrade(baseAgeInMonths);
			auto upgrade = mod->getDeployment(upgradeId, false);
			if (upgrade && upgrade != alienBase->getDeployment())
			{
				std::ostringstream ss;
				ss << "month: " << month;
				ss << " baseId: " << alienBase->getId();
				ss << " baseType: " << alienBase->getType();
				if (alienBase->getDeployment()->resetAlienBaseAgeAfterUpgrade() || upgrade->resetAlienBaseAge())
				{
					// reset base age to zero
					alienBase->setStartMonth(month);
					ss << "; base age was reset;";
				}
				ss << " old deployment: " << alienBase->getDeployment()->getType();
				alienBase->setDeployment(upgrade);
				ss << " new deployment: " << alienBase->getDeployment()->getType();
				auto* upgradeRace = mod->getAlienRace(upgrade->getUpgradeRace(), false);
				if (upgradeRace)
				{
					ss << " old race: " << alienBase->getAlienRace();
					alienBase->setAlienRace(upgradeRace->getId());
					ss << " new race: " << alienBase->getAlienRace();
				}
				if (Options::oxceGeoscapeDebugLogMaxEntries > 0)
				{
					save->getGeoscapeDebugLog().push_back(ss.str());
				}
			}
		}
	}
}


/**
 * Processes a directive to start up a mission, if possible.
 * @param command the directive from which to read information.
 * @return whether the command successfully produced a new mission.
 */
bool GeoscapeState::processCommand(RuleMissionScript *command)
{
	SavedGame *save = getGame()->getSavedGame();
	AlienStrategy &strategy = save->getAlienStrategy();
	Mod *mod = getGame()->getMod();
	int month = getGame()->getSavedGame()->getMonthsPassed();
	std::string targetRegion;
	const RuleAlienMission *missionRules;
	std::string missionType;
	std::string missionRace;
	int targetZoneNumber = -1;
	int targetAreaNumber = -1;

	// terror mission type deal? this will require special handling.
	if (command->getSiteType())
	{
		// we know for a fact that this command has mission weights defined, otherwise this flag could not be set.
		missionType = command->generate(month, GEN_MISSION);
		std::vector<std::string> missions = command->getMissionTypes(month);
		int maxMissions = (int)missions.size();
		bool targetBase = RNG::percent(command->getTargetBaseOdds());
		int currPos = 0;
		for (; currPos != maxMissions; ++currPos)
		{
			if (missions[currPos] == missionType)
			{
				break;
			}
		}

		// let's build a list of regions with spawn zones to pick from
		std::vector<std::pair<std::string, int> > validAreas;

		// this is actually a bit of a cheat, we ARE using the mission weights as defined, but we'll try them all if the one we pick first isn't valid.
		for (int h = 0; h != maxMissions; ++h)
		{
			// we'll use the regions listed in the command, if any, otherwise check all the regions in the ruleset looking for matches
			std::vector<std::string> regions = (command->hasRegionWeights()) ? command->getRegions(month) : mod->getRegionsList();
			missionRules = mod->getAlienMission(missionType, true);
			targetZoneNumber = missionRules->getSpawnZone();

			if (targetBase)
			{
				std::vector<std::string> regionsToKeep;
				//if we're targetting a base, we ignore regions that don't contain bases, simple.
				for (const Base& xcomBase : getRegistry().list<Base>())
				{
					if (const Region* region = AreaSystem::locateValue<const Region>(xcomBase))
					{
						regionsToKeep.push_back(region->getRules()->getType());
					}
				}
				auto inKeptRegions = [&regionsToKeep](const std::string& regionName) {
					return std::ranges::find(regionsToKeep, regionName) == regionsToKeep.end();
				};
				regions.erase(std::remove_if(regions.begin(), regions.end(), inKeptRegions), regions.end());
			}

			// -----------------------------------------------------------
			// Summary of mission site spawning algorithms (objective: 3)
			// -----------------------------------------------------------

			// Type 1:
			// - no UFOs involved
			// - only 1 wave
			// - the wave specifies the alien deployment directly (e.g. `ufo: STR_ARTIFACT_SITE_P1 # spawn this site directly`)
			// - example (1): STR_ALIEN_ARTIFACT (TFTD)
			// Support for non-point areas: yes, without any additional ruleset changes required
			const MissionWave& wave = missionRules->getWave(0);
			bool spawnMissionSiteDirectly = (mod->getDeployment(wave.ufoType) && !mod->getUfo(wave.ufoType) && !mod->getDeployment(wave.ufoType)->getMarkerName().empty());

			// Type 2:
			// - no UFOs involved
			// - only 1 wave
			// - the wave does NOT specify the alien deployment directly (e.g. `ufo: dummy #don't spawn a ufo, we only want the site`)
			//   -> option A: alien deployment is chosen randomly = from the area's texture definition
			//   -> option B: alien deployment is specified by the mission's `siteType` (overrides option A if both are defined)
			// - example (2A): STR_ALIEN_SHIP_ATTACK (TFTD)
			// - example (2B): none in vanilla, only mods
			// Support for non-point areas: yes, without any additional ruleset changes required
			// bool spawnMissionSiteByTexture = area.texture < 0
			// bool spawnMissionSiteBySiteType = !missionRules->getSiteType().empty();

			// Type 3:
			// - with UFOs waves
			// - only 1 wave with `objective: true`
			// - the wave does NOT specify the alien deployment (because it already specifies the UFO type)
			//   -> option A: alien deployment is chosen randomly = from the area's texture definition
			//   -> option B: alien deployment is specified by the mission's `siteType` (overrides option A if both are defined)
			// - example (3A): STR_ALIEN_SURFACE_ATTACK (TFTD)
			// - example (3B): none in vanilla, only mods
			// Support for non-point areas: yes, but it is recommended to use one more wave attribute: `objectiveOnTheLandingSite: true`
			//   -> false: UFO always lands in the top-left corner of the area; site spawns randomly inside the area
			//   ->  true: UFO lands randomly inside the area; site spawns exactly on the UFO landing site
			// bool spawnMissionSiteByTexture = area.texture < 0
			bool spawnMissionSiteBySiteType = !missionRules->getSiteType().empty();

			// -----------------------------------------------
			// End of the summary
			// -----------------------------------------------

			for (auto regionNameIt = regions.begin(); regionNameIt != regions.end();)
			{
				// we don't want the same mission running in any given region twice simultaneously, so prune the list as needed.
				bool processThisRegion = true;
				for (auto* am : save->getAlienMissions())
				{
					if (am->getRules().getType() == missionRules->getType() && am->getRegion() == (*regionNameIt))
					{
						processThisRegion = false;
						break;
					}
				}
				if (!processThisRegion)
				{
					regionNameIt = regions.erase(regionNameIt);
					continue;
				}
				// ok, we found a region that doesn't have our mission in it, let's see if it has an appropriate landing zone.
				// if it does, let's add it to our list of valid areas, taking note of which mission area(s) matched.
				RuleRegion *region = mod->getRegion((*regionNameIt), true);
				if ((int)(region->getMissionZones().size()) > targetZoneNumber)
				{
					std::vector<MissionArea> areas = region->getMissionZones()[targetZoneNumber].areas;
					int counter = 0;
					for (const auto& area : areas)
					{
						// validMissionLocation checks to make sure this city/whatever hasn't been used by the last n missions using this varName
						// this prevents the same location getting hit more than once every n missions.
						if (area.isPoint() && strategy.validMissionLocation(command->getVarName(), region->getType(), counter))
						{
							validAreas.push_back(std::make_pair(region->getType(), counter));
						}
						else if (!area.isPoint() && (area.texture < 0 || spawnMissionSiteBySiteType || spawnMissionSiteDirectly))
						{
							validAreas.push_back(std::make_pair(region->getType(), counter));
						}
						counter++;
					}
				}
				++regionNameIt;
			}

			// oh bother, we couldn't find anything valid, this mission won't run this month.
			if (validAreas.empty())
			{
				if (maxMissions > 1 && ++currPos == maxMissions)
				{
					currPos = 0;
				}
				missionType = missions[currPos];
			}
			else
			{
				break;
			}
		}

		if (validAreas.empty())
		{
			// now we're in real trouble, we've managed to make it out of the loop and we still don't have any valid choices
			// this command cannot run this month, we have failed, forgive us senpai.
			return false;
		}

		// everything went according to plan: we can now pick a city/whatever to attack.
		while (targetAreaNumber == -1)
		{
			if (command->hasRegionWeights())
			{
				// if we have a weighted region list, we know we have at least one valid choice for this mission
				targetRegion = command->generate(month, GEN_REGION);
			}
			else
			{
				// if we don't have a weighted list, we'll select a region at random from the ruleset,
				// validate that it's in our list, and pick one of its cities at random
				// this will give us an even distribution between regions regardless of the number of cities.
				targetRegion = mod->getRegionsList().at(RNG::generate(0, (int)mod->getRegionsList().size() - 1));
			}

			// we need to know the range of the region within our vector, in order to randomly select a city from it
			int min = -1;
			int max = -1;
			int curr = 0;
			for (const auto& pair : validAreas)
			{
				if (pair.first == targetRegion)
				{
					if (min == -1)
					{
						min = curr;
					}
					max = curr;
				}
				else if (min > -1)
				{
					// if we've stopped detecting matches, we're done looking.
					break;
				}
				++curr;
			}
			if (min != -1)
			{
				// we have our random range, we can make a selection, and we're done.
				targetAreaNumber = validAreas[RNG::generate(min, max)].second;
			}
		}
		// now add that city to the list of sites we've hit, store the array, etc.
		strategy.addMissionLocation(command->getVarName(), targetRegion, targetAreaNumber, command->getRepeatAvoidance());
	}
	else if (RNG::percent(command->getTargetBaseOdds()))
	{
		// build a list of the mission types we're dealing with, if any
		std::vector<std::string> types = command->getMissionTypes(month);
		// now build a list of regions with bases in.
		std::vector<std::string> regionsMaster;
		for (const Base& xcomBase : getRegistry().list<Base>())
		{
			if (const Region* region = AreaSystem::locateValue<const Region>(xcomBase))
			{
				regionsMaster.push_back(region->getRules()->getType());
			}
		}
		// no defined mission types? then we'll prune the region list to ensure we only have a region that can generate a mission.
		if (types.empty())
		{
			for (auto regionNameIt = regionsMaster.begin(); regionNameIt != regionsMaster.end();)
			{
				if (!strategy.validMissionRegion((*regionNameIt)))
				{
					regionNameIt = regionsMaster.erase(regionNameIt);
					continue;
				}
				++regionNameIt;
			}
			// no valid missions in any base regions? oh dear, i guess we failed.
			if (regionsMaster.empty())
			{
				return false;
			}
			// pick a random region from our list
			targetRegion = regionsMaster[RNG::generate(0, static_cast<int>(regionsMaster.size()-1))];
		}
		else
		{
			// we don't care about regional mission distributions, we're targetting a base with whatever mission we pick, so let's pick now
			// we'll iterate the mission list, starting at a random point, and wrapping around to the beginning
			int max = (int)types.size();
			int entry = RNG::generate(0,  max - 1);
			std::vector<std::string> regions;

			for (int i = 0; i != max; ++i)
			{
				regions = regionsMaster;
				for (auto* am : save->getAlienMissions())
				{
					// if the mission types match
					if (types[entry] == am->getRules().getType())
					{
						for (auto regionNameIt = regions.begin(); regionNameIt != regions.end();)
						{
							// and the regions match
							if ((*regionNameIt) == am->getRegion())
							{
								// prune the entry from the list
								regionNameIt = regions.erase(regionNameIt);
								continue;
							}
							++regionNameIt;
						}
					}
				}

				// we have a valid list of regions containing bases, pick one.
				if (!regions.empty())
				{
					missionType = types[entry];
					targetRegion = regions[RNG::generate(0, (int)regions.size()-1)];
					break;
				}
				// otherwise, try the next mission in the list.
				if (max > 1 && ++entry == max)
				{
					entry = 0;
				}
			}
		}
	}
	// now the easy stuff
	else if (!command->hasRegionWeights())
	{
		// no regionWeights means we pick from the table
		targetRegion = strategy.chooseRandomRegion(mod);
	}
	else
	{
		// otherwise, let the command dictate the region.
		targetRegion = command->generate(month, GEN_REGION);
	}

	if (targetRegion.empty())
	{
		// something went horribly wrong, we should have had at LEAST a region by now.
		return false;
	}

	// we're bound to end up with typos, so let's throw an exception instead of simply returning false
	// that way, the modder can fix their mistake
	if (mod->getRegion(targetRegion) == 0)
	{
		throw Exception("Error processing mission script named: " + command->getType() + ", region named: " + targetRegion + " is not defined");
	}

	if (missionType.empty()) // ie: not a terror mission, not targetting a base, or otherwise not already chosen
	{
		if (!command->hasMissionWeights())
		{
			// no weights means let the strategy pick
			missionType = strategy.chooseRandomMission(targetRegion);
		}
		else
		{
			// otherwise the command gives us the weights.
			missionType = command->generate(month, GEN_MISSION);
		}
	}

	if (missionType.empty())
	{
		// something went horribly wrong, we didn't manage to choose a mission type
		return false;
	}

	missionRules = mod->getAlienMission(missionType);

	// we're bound to end up with typos, so let's throw an exception instead of simply returning false
	// that way, the modder can fix their mistake
	if (missionRules == 0)
	{
		throw Exception("Error processing mission script named: " + command->getType() + ", mission type: " + missionType + " is not defined");
	}

	// do i really need to comment this? shouldn't it be obvious what's happening here?
	if (!command->hasRaceWeights())
	{
		missionRace = missionRules->generateRace(month);
	}
	else
	{
		missionRace = command->generate(month, GEN_RACE);
	}

	if (missionRace.empty())
	{
		throw Exception("Error processing mission script named: " + command->getType() + ", mission type: " + missionType + " has no available races");
	}

	// we're bound to end up with typos, so let's throw an exception instead of simply returning false
	// that way, the modder can fix their mistake
	if (mod->getAlienRace(missionRace) == 0)
	{
		throw Exception("Error processing mission script named: " + command->getType() + ", race: " + missionRace + " is not defined");
	}

	// ok, we've derived all the variables we need to start up our mission, let's do magic to turn those values into a mission
	AlienMission *mission = new AlienMission(*missionRules);
	mission->setRace(missionRace);
	mission->setId(getGame()->getSavedGame()->getId("ALIEN_MISSIONS"));
	mission->setRegion(targetRegion, *getGame()->getMod());
	mission->setMissionSiteZoneArea(targetAreaNumber);
	strategy.addMissionRun(command->getVarName());
	mission->start(*getGame(), *_globe, command->getDelay());
	getGame()->getSavedGame()->getAlienMissions().push_back(mission);
	// if this flag is set, we want to delete it from the table so it won't show up again until the schedule resets.
	if (command->getUseTable())
	{
		strategy.removeMission(targetRegion, missionType);
	}

	if (Options::oxceGeoscapeDebugLogMaxEntries > 0)
	{
		std::ostringstream ss;
		ss << "month: " << month;
		ss << " script: " << command->getType();
		ss << " id: " << mission->getId();
		ss << " type: " << mission->getRules().getType();
		ss << " race: " << mission->getRace();
		ss << " region: " << targetRegion; /* << " / " << mission->getRegion() */
		ss << " targetZone: " << targetZoneNumber;
		ss << " targetArea: " << targetAreaNumber;
		save->getGeoscapeDebugLog().push_back(ss.str());
	}

	// we did it, we can go home now.
	return true;

}

/**
 * Handler for clicking on a timer button.
 * @param action pointer to the mouse action.
 */
void GeoscapeState::btnTimerClick(Action *action)
{
	SDL_Event ev{};
	ev.type = SDL_MOUSEBUTTONDOWN;
	ev.button.button = SDL_BUTTON_LEFT;
	Action a = Action(&ev, 0.0, 0.0, 0, 0);
	action->getSender()->mousePress(&a, this);
}

/**
 * Updates the scale.
 * @param dX delta of X;
 * @param dY delta of Y;
 */
void GeoscapeState::resize(int &dX, int &dY)
{
	if (getGame()->getSavedGame()->getSavedBattle())
		return;
	dX = Options::baseXResolution;
	dY = Options::baseYResolution;
	int divisor = 1;
	double pixelRatioY = 1.0;

	if (Options::nonSquarePixelRatio)
	{
		pixelRatioY = 1.2;
	}
	switch (Options::geoscapeScale)
	{
	case SCALE_SCREEN_DIV_6:
		divisor = 6;
		break;
	case SCALE_SCREEN_DIV_5:
		divisor = 5;
		break;
	case SCALE_SCREEN_DIV_4:
		divisor = 4;
		break;
	case SCALE_SCREEN_DIV_3:
		divisor = 3;
		break;
	case SCALE_SCREEN_DIV_2:
		divisor = 2;
		break;
	case SCALE_SCREEN:
		break;
	default:
		dX = 0;
		dY = 0;
		return;
	}

	Options::baseXResolution = std::max(Screen::ORIGINAL_WIDTH, Options::displayWidth / divisor);
	Options::baseYResolution = std::max(Screen::ORIGINAL_HEIGHT, (int)(Options::displayHeight / pixelRatioY / divisor));

	dX = Options::baseXResolution - dX;
	dY = Options::baseYResolution - dY;

	_globe->resize();

	for (entt::entity surfaceEnt : _surfaces)
	{
		Surface* surface = getRegistry().raw().get<SurfaceComponent>(surfaceEnt).getSurface();
		if (surface != _globe)
		{
			surface->setX(surface->getX() + dX);
			surface->setY(surface->getY() + dY/2);
		}
	}

	_bg->setX((_globe->getWidth() - _bg->getWidth()) / 2);
	_bg->setY((_globe->getHeight() - _bg->getHeight()) / 2);

	int height = (Options::baseYResolution - Screen::ORIGINAL_HEIGHT) / 2 + 10;
	_sideTop->setHeight(height);
	_sideTop->setY(_sidebar->getY() - height - 1);
	_sideBottom->setHeight(height);
	_sideBottom->setY(_sidebar->getY() + _sidebar->getHeight() + 1);

	_sideLine->setHeight(Options::baseYResolution);
	_sideLine->setY(0);
	_sideLine->drawRect(0, 0, _sideLine->getWidth(), _sideLine->getHeight(), 15);
}
bool GeoscapeState::buttonsDisabled()
{
	return _zoomInEffectTimer->isRunning() || _zoomOutEffectTimer->isRunning();
}

void GeoscapeState::updateSlackingIndicator()
{
	if (!Options::oxceEnableSlackingIndicator)
		return;

	int scientistsSlacking = getRegistry().totalBy<Base, int>(&Base::getAvailableScientists);
	int engineersSlacking = getRegistry().totalBy<Base, int>(&Base::getAllocatedEngineers);
	LocalizedText slackingText = scientistsSlacking > 0 || engineersSlacking > 0 ? tr("STR_SLACKING_INDICATOR").arg(scientistsSlacking).arg(engineersSlacking)
																				 : tr("");
	_txtSlacking->setText(slackingText);
}

void GeoscapeState::cbxRegionChange(Action *)
{
	size_t index = _cbxRegion->getSelected();
	if (index < 1)
	{
		getGame()->getSavedGame()->debugRegion = entt::null;
	}
	else
	{
		getGame()->getSavedGame()->debugRegion = _regionDebugComboBoxIndexToEntity[index - 1];
	}
	updateZoneInfo();
}

void GeoscapeState::cbxZoneChange(Action *)
{
	getGame()->getSavedGame()->debugZone = _cbxZone->getSelected();
	updateZoneInfo();
}

void GeoscapeState::cbxAreaChange(Action *)
{
	getGame()->getSavedGame()->debugArea = _cbxArea->getSelected();
	updateZoneInfo();
}

void GeoscapeState::updateZoneInfo()
{
	std::ostringstream ss;
	auto* save = getGame()->getSavedGame();
	if (save->debugRegion != entt::null)
	{
		const RuleRegion* regionRule = getRegistry().raw().get<Region>(save->debugRegion).getRules();
		if (save->debugType >= 1)
		{
			ss << "region: " << tr(regionRule->getType()) << " [" << regionRule->getType() << "]" << std::endl;
		}
		if (save->debugType == 2)
		{
			if (save->debugZone > 0 && save->debugZone <= regionRule->getMissionZones().size())
			{
				auto& selectedZone = regionRule->getMissionZones().at(save->debugZone - 1);
				ss << "zone: " << save->debugZone - 1 << std::endl;
				if (save->debugArea > 0 && save->debugArea <= selectedZone.areas.size())
				{
					auto& selectedArea = selectedZone.areas.at(save->debugArea - 1);
					ss << "area: " << save->debugArea - 1;
					ss << ", texture: " << selectedArea.texture;
					ss << ", name: " << tr(selectedArea.name) << " [" << selectedArea.name << "]" << std::endl;
					if (selectedArea.isPoint())
					{
						ss << "point = [" << selectedArea.lonMin / M_PI * 180 << ", " << selectedArea.latMin / M_PI * 180 << "]";
						int texture, shade;
						_globe->getPolygonTextureAndShade(selectedArea.lonMin, selectedArea.latMin, &texture, &shade);
						ss << ", globe texture: " << texture;
					}
					else
					{
						ss << "rect = [" << selectedArea.lonMin / M_PI * 180 << ", " << selectedArea.lonMax / M_PI * 180;
						ss << ", " << selectedArea.latMin / M_PI * 180 << ", " << selectedArea.latMax / M_PI * 180 << "]";
					}
				}
				else
				{
					ss << "total areas: " << selectedZone.areas.size() << std::endl;
				}
			}
			else
			{
				ss << "total zones: " << regionRule->getMissionZones().size() << std::endl;
			}
		}
	}
	_txtDebug->setText(ss.str());
}

void GeoscapeState::cbxCountryChange(Action *)
{
	size_t index = _cbxCountry->getSelected();
	if (index < 1)
	{
		getGame()->getSavedGame()->debugCountry = entt::null;
	}
	else
	{
		getGame()->getSavedGame()->debugCountry = _countryDebugComboBoxIndexToEntity[index - 1];
	}

	std::string debugCountryText;
	auto* save = getGame()->getSavedGame();
	if (save->debugCountry != entt::null && save->debugType == 0)
	{
		const RuleCountry* countryRule = getRegistry().raw().get<Country>(save->debugCountry).getRules();
		debugCountryText = std::format("country: {} [{}]", tr(countryRule->getType()).c_str(), countryRule->getType());
	}
	_txtDebug->setText(debugCountryText);
}

}
