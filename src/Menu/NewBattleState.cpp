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
#include "NewBattleState.h"
#include <cmath>
#include <algorithm>
#include <yaml-cpp/yaml.h>
#include "../Engine/Game.h"
#include "../Entity/Game/BaseFactory.h"
#include "../Entity/Game/UfoFactory.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleItem.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextEdit.h"
#include "../Interface/TextList.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/ComboBox.h"
#include "../Interface/Slider.h"
#include "../Interface/Frame.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/Craft.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/Soldier.h"
#include "../Battlescape/BattlescapeGenerator.h"
#include "../Battlescape/BriefingState.h"
#include "../Savegame/Ufo.h"
#include "../Savegame/MissionSite.h"
#include "../Savegame/AlienBase.h"
#include "../Mod/RuleCraft.h"
#include "../Mod/RuleTerrain.h"
#include "../Mod/AlienDeployment.h"
#include "../Engine/RNG.h"
#include "../Engine/Action.h"
#include "../Engine/Options.h"
#include "../Engine/Logger.h"
#include "../Basescape/CraftInfoState.h"
#include "../Engine/CrossPlatform.h"
#include "../Mod/RuleAlienMission.h"
#include "../Mod/AlienRace.h"
#include "../Mod/RuleGlobe.h"

#include "../Entity/Engine/Surface.h"
#include "../Entity/Interface/Interface.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the New Battle window.
 * @param game Pointer to the core game.
 */
NewBattleState::NewBattleState()
	: State("NewBattleState", true), _craft(0), _selectType(NewBattleSelectType::MISSION), _isRightClick(false),
	_depthVisible(false), _globeTextureVisible(false), _selectedGlobeTexture(0)
{
	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();

	// Create objects
	_window = factory.createWindow("windowName", this, 320, 200, 0, 0, WindowPopup::POPUP_BOTH);
	_btnQuickSearch = new TextEdit(this, 48, 9, 264, 183);
	_txtTitle = new Text(320, 17, 0, 9);

	_txtMapOptions = new Text(148, 9, 8, 68);
	_frameLeft = new Frame(148, 96, 8, 78);
	_txtAlienOptions = new Text(148, 9, 164, 68);
	_frameRight = new Frame(148, 96, 164, 78);

	_txtMission = new Text(100, 9, 8, 30);
	_cbxMission = new ComboBox(this, 214, 16, 98, 26);
	_btnMission = new TextButton(16, 16, 81, 26);

	_txtCraft = new Text(100, 9, 8, 50);
	_cbxCraft = new ComboBox(this, 106, 16, 98, 46);
	_btnEquip = new TextButton(106, 16, 206, 46);

	_txtDarkness = new Text(120, 9, 22, 83);
	_slrDarkness = new Slider(120, 16, 22, 93);

	_txtTerrain = new Text(120, 9, 22, 113);
	_cbxTerrain = new ComboBox(this, 120, 16, 22+9, 123);
	_btnTerrain = new TextButton(16, 16, 5+9, 123);

	_txtDepth = new Text(120, 9, 22, 143);
	_slrDepth = new Slider(120, 16, 22, 153);

	_txtGlobeTexture = new Text(120, 9, 22, 143);
	_btnGlobeTexture = new TextButton(120, 16, 22, 153);
	_btnGlobeTextureToggle = new TextButton(16, 16, 145, 153);

	_txtDifficulty = new Text(120, 9, 178, 83);
	_cbxDifficulty = new ComboBox(this, 120, 16, 178, 93);

	_txtAlienRace = new Text(120, 9, 178, 113);
	_cbxAlienRace = new ComboBox(this, 120, 16, 178+9, 123);
	_btnAlienRace = new TextButton(16, 16, 161+9, 123);

	_txtAlienTech = new Text(120, 9, 178, 143);
	_slrAlienTech = new Slider(120, 16, 178, 153);

	_btnOk = new TextButton(100, 16, 8, 176);
	_btnCancel = new TextButton(100, 16, 110, 176);
	_btnRandom = new TextButton(100, 16, 212, 176);

	_lstSelect = new TextList(288, 144, 8, 28);

	// Set palette
	setInterface("newBattleMenu");

	add(_window, "window", "newBattleMenu");
	add(_btnQuickSearch, "button1", "newBattleMenu");
	add(_txtTitle, "heading", "newBattleMenu");
	add(_txtMapOptions, "heading", "newBattleMenu");
	add(_frameLeft, "frames", "newBattleMenu");
	add(_txtAlienOptions, "heading", "newBattleMenu");
	add(_frameRight, "frames", "newBattleMenu");

	add(_txtMission, "text", "newBattleMenu");
	add(_txtCraft, "text", "newBattleMenu");
	add(_btnEquip, "button1", "newBattleMenu");

	add(_txtDarkness, "text", "newBattleMenu");
	add(_slrDarkness, "button1", "newBattleMenu");
	add(_txtDepth, "text", "newBattleMenu");
	add(_slrDepth, "button1", "newBattleMenu");
	add(_txtGlobeTexture, "text", "newBattleMenu");
	add(_btnGlobeTexture, "button1", "newBattleMenu");
	add(_btnGlobeTextureToggle, "button1", "newBattleMenu");
	add(_txtTerrain, "text", "newBattleMenu");
	add(_txtDifficulty, "text", "newBattleMenu");
	add(_txtAlienRace, "text", "newBattleMenu");
	add(_txtAlienTech, "text", "newBattleMenu");
	add(_slrAlienTech, "button1", "newBattleMenu");

	add(_btnOk, "button2", "newBattleMenu");
	add(_btnCancel, "button2", "newBattleMenu");
	add(_btnRandom, "button2", "newBattleMenu");

	add(_btnMission, "button1", "newBattleMenu");
	add(_btnTerrain, "button1", "newBattleMenu");
	add(_btnAlienRace, "button1", "newBattleMenu");

	add(_lstSelect, "list", "newBattleMenu");

	add(_cbxTerrain, "button1", "newBattleMenu");
	add(_cbxAlienRace, "button1", "newBattleMenu");
	add(_cbxDifficulty, "button1", "newBattleMenu");
	add(_cbxCraft, "button1", "newBattleMenu");
	add(_cbxMission, "button1", "newBattleMenu");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "newBattleMenu");

	_txtTitle->setAlign(TextHAlign::ALIGN_CENTER);
	_txtTitle->setBig();
	_txtTitle->setText(tr("STR_MISSION_GENERATOR"));

	_txtMapOptions->setText(tr("STR_MAP_OPTIONS"));

	_frameLeft->setThickness(3);

	_txtAlienOptions->setText(tr("STR_ALIEN_OPTIONS"));

	_frameRight->setThickness(3);

	_txtMission->setText(tr("STR_MISSION"));

	_txtCraft->setText(tr("STR_CRAFT"));

	_txtDarkness->setText(tr("STR_MAP_DARKNESS"));

	_txtDepth->setText(tr("STR_MAP_DEPTH"));

	_txtTerrain->setText(tr("STR_MAP_TERRAIN"));

	_txtGlobeTexture->setText(tr("STR_GLOBE_TEXTURE"));
	_txtGlobeTexture->setVisible(false);

	_txtDifficulty->setText(tr("STR_DIFFICULTY"));

	_txtAlienRace->setText(tr("STR_ALIEN_RACE"));

	_txtAlienTech->setText(tr("STR_ALIEN_TECH_LEVEL"));

	if (Options::debug)
	{
		_missionTypes = getGame()->getMod()->getDeploymentsList();
	}
	else
	{
		_missionTypes.reserve(getGame()->getMod()->getDeploymentsList().size());
		for (auto &deploymentName : getGame()->getMod()->getDeploymentsList())
		{
			auto depl = getGame()->getMod()->getDeployment(deploymentName);
			if (depl && !depl->isHidden())
			{
				_missionTypes.push_back(deploymentName);
			}
		}
	}
	// ignore, hardcoded
	{
		auto itr = std::find(_missionTypes.begin(), _missionTypes.end(), RuleCraft::DEFAULT_CRAFT_DEPLOYMENT_PREVIEW);
		if (itr != _missionTypes.end())
		{
			_missionTypes.erase(itr);
		}
	}
	_cbxMission->setOptions(_missionTypes, true);
	_cbxMission->onChange((ActionHandler)&NewBattleState::cbxMissionChange);

	for (auto& craftType : getGame()->getMod()->getCraftsList())
	{
		RuleCraft *rule = getGame()->getMod()->getCraft(craftType);
		if (rule->isForNewBattle())
		{
			_crafts.push_back(craftType);
		}
	}
	_cbxCraft->setOptions(_crafts, true);
	_cbxCraft->onChange((ActionHandler)&NewBattleState::cbxCraftChange);

	_slrDarkness->setRange(0, 15);

	_slrDepth->setRange(1, 3);

	_cbxTerrain->onChange((ActionHandler)&NewBattleState::cbxTerrainChange);

	for (auto& pair : getGame()->getMod()->getGlobe()->getTexturesRaw())
	{
		if (pair.first >= 0)
		{
			_globeTextures.push_back("GLOBE_TEXTURE_" + std::to_string(pair.first));
			_globeTextureIDs.push_back(pair.first);
		}
	}

	_btnGlobeTexture->setText(tr(_globeTextures[_selectedGlobeTexture]));
	_btnGlobeTexture->onMouseClick((ActionHandler)&NewBattleState::btnGlobeTextureChange);
	_btnGlobeTexture->onMouseClick((ActionHandler)&NewBattleState::btnGlobeTextureChange, SDL_BUTTON_RIGHT);
	_btnGlobeTexture->setVisible(false);

	_btnGlobeTextureToggle->setText("*");
	_btnGlobeTextureToggle->onMouseClick((ActionHandler)&NewBattleState::btnGlobeTextureToggle);
	_btnGlobeTextureToggle->setVisible(false);

	std::vector<std::string> difficulty;
	difficulty.push_back(tr("STR_1_BEGINNER"));
	difficulty.push_back(tr("STR_2_EXPERIENCED"));
	difficulty.push_back(tr("STR_3_VETERAN"));
	difficulty.push_back(tr("STR_4_GENIUS"));
	difficulty.push_back(tr("STR_5_SUPERHUMAN"));
	_cbxDifficulty->setOptions(difficulty);

	_slrAlienTech->setRange(0, (int)getGame()->getMod()->getAlienItemLevels().size()-1);
	if (getGame()->getMod()->getAlienItemLevels().size() <= 1)
	{
		_slrAlienTech->setVisible(false);
		_txtAlienTech->setVisible(false);
	}

	_btnEquip->setText(tr("STR_EQUIP_CRAFT"));
	_btnEquip->onMouseClick((ActionHandler)&NewBattleState::btnEquipClick);

	_btnRandom->setText(tr("STR_RANDOMIZE"));
	_btnRandom->onMouseClick((ActionHandler)&NewBattleState::btnRandomClick);

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&NewBattleState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&NewBattleState::btnOkClick, Options::keyOk);

	_btnCancel->setText(tr("STR_CANCEL"));
	_btnCancel->onMouseClick((ActionHandler)&NewBattleState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&NewBattleState::btnCancelClick, Options::keyCancel);

	load();

	// -------------------------------

	bool showExtraButtons = _missionTypes.size() > TFTD_DEPLOYMENTS;
	if (!showExtraButtons)
	{
		_cbxTerrain->setX(_txtTerrain->getX());
		_cbxAlienRace->setX(_txtAlienRace->getX());
	}

	_btnMission->setText("...");
	_btnMission->onMouseClick((ActionHandler)&NewBattleState::btnMissionChange);
	_btnMission->onMouseClick((ActionHandler)&NewBattleState::btnMissionChange, SDL_BUTTON_RIGHT);
	_btnMission->setVisible(showExtraButtons);

	_btnTerrain->setText("...");
	_btnTerrain->onMouseClick((ActionHandler)&NewBattleState::btnTerrainChange);
	_btnTerrain->onMouseClick((ActionHandler)&NewBattleState::btnTerrainChange, SDL_BUTTON_RIGHT);
	_btnTerrain->setVisible(showExtraButtons);

	_btnAlienRace->setText("...");
	_btnAlienRace->onMouseClick((ActionHandler)&NewBattleState::btnAlienRaceChange);
	_btnAlienRace->onMouseClick((ActionHandler)&NewBattleState::btnAlienRaceChange, SDL_BUTTON_RIGHT);
	_btnAlienRace->setVisible(showExtraButtons);

	_lstSelect->setColumns(1, 280);
	_lstSelect->setBackground(_window);
	_lstSelect->setAlign(TextHAlign::ALIGN_CENTER);
	_lstSelect->setMargin(8);
	_lstSelect->setSelectable(true);
	_lstSelect->onMouseClick((ActionHandler)&NewBattleState::lstSelectClick);
	_lstSelect->onMouseClick((ActionHandler)&NewBattleState::lstSelectClick, SDL_BUTTON_RIGHT);
	_lstSelect->onMouseClick((ActionHandler)&NewBattleState::lstSelectClick, SDL_BUTTON_MIDDLE);
	_lstSelect->setVisible(false);

	_btnQuickSearch->setText(""); // redraw
	_btnQuickSearch->onEnter((ActionHandler)&NewBattleState::btnQuickSearchApply);
	_btnQuickSearch->setVisible(false);

	_btnCancel->onKeyboardRelease((ActionHandler)&NewBattleState::btnQuickSearchToggle, Options::keyToggleQuickSearch);
}

/**
 *
 */
NewBattleState::~NewBattleState()
{

}

/**
 * Resets the menu music and savegame
 * when coming back from the battlescape.
 */
void NewBattleState::init()
{
	State::init();

	if (_craft == 0)
	{
		load();
	}
}

/**
 * Loads new battle data from a YAML file.
 * @param filename YAML filename.
 */
void NewBattleState::load(const std::string &filename)
{
	std::string s = Options::getMasterUserFolder() + filename + ".cfg";
	if (!CrossPlatform::fileExists(s))
	{
		initSave();
	}
	else
	{
		try
		{
			YAML::Node doc = YAML::Load(*CrossPlatform::readFile(s));
			_cbxMission->setSelected(std::min(doc["mission"].as<size_t>(0), _missionTypes.size() - 1));
			cbxMissionChange(0);
			_cbxCraft->setSelected(std::min(doc["craft"].as<size_t>(0), _crafts.size() - 1));
			_slrDarkness->setValue((int)doc["darkness"].as<size_t>(0));
			_cbxTerrain->setSelected(std::min(doc["terrain"].as<size_t>(0), _terrainTypes.size() - 1));
			cbxTerrainChange(0);
			{
				_selectedGlobeTexture = std::min(doc["globeTexture"].as<size_t>(0), _globeTextures.size() - 1);
				_btnGlobeTexture->setText(tr(_globeTextures[_selectedGlobeTexture]));
			}
			_cbxAlienRace->setSelected(std::min(doc["alienRace"].as<size_t>(0), _alienRaces.size() - 1));
			_cbxDifficulty->setSelected(doc["difficulty"].as<size_t>(0));
			_slrAlienTech->setValue((int)doc["alienTech"].as<size_t>(0));

			if (doc["base"])
			{
				const Mod *mod = getGame()->getMod();
				SavedGame *save = new SavedGame();

				Base& newBase = getRegistry().getService<BaseFactory>().create(*mod, doc["base"], save, false).get<Base>();

				// Add research
				for (auto& pair : mod->getResearchMap())
				{
					save->addFinishedResearchSimple(pair.second);
				}

				// Generate items
				newBase.getStorageItems()->clear();
				for (auto& itemType : mod->getItemsList())
				{
					RuleItem *rule = getGame()->getMod()->getItem(itemType);
					if (rule->getBattleType() != BT_CORPSE && rule->isRecoverable())
					{
						newBase.getStorageItems()->addItem(rule, 1);
					}
				}

				// Fix invalid contents
				if (newBase.getCrafts().empty())
				{
					std::string craftType = _crafts[_cbxCraft->getSelected()];
					_craft = new Craft(getGame()->getMod()->getCraft(craftType), &newBase, save->getId(craftType));
					newBase.getCrafts().push_back(_craft);
				}
				else
				{
					_craft = newBase.getCrafts().front();
				}

				getGame()->setSavedGame(save);
			}
			else
			{
				initSave();
			}
		}
		catch (YAML::Exception &e)
		{
			Log(LOG_WARNING) << e.what();
			initSave();
		}
	}
}

/**
 * Saves new battle data to a YAML file.
 * @param filename YAML filename.
 */
void NewBattleState::save(const std::string &filename)
{
	YAML::Emitter out;
	YAML::Node node;
	node["mission"] = _cbxMission->getSelected();
	node["craft"] = _cbxCraft->getSelected();
	node["darkness"] = _slrDarkness->getValue();
	node["terrain"] = _cbxTerrain->getSelected();
	node["globeTexture"] = _selectedGlobeTexture;
	node["alienRace"] = _cbxAlienRace->getSelected();
	node["difficulty"] = _cbxDifficulty->getSelected();
	node["alienTech"] = _slrAlienTech->getValue();
	node["base"] = getRegistry().front<Base>().try_get<Base>()->save();
	out << node;

	std::string filepath = Options::getMasterUserFolder() + filename + ".cfg";
	if (!CrossPlatform::writeFile(filepath, out.c_str()))
	{
		Log(LOG_WARNING) << "Failed to save " << filepath;
		return;
	}
}

/**
 * Initializes a new savegame with
 * everything available.
 */
void NewBattleState::initSave()
{
	const Mod& mod = *getGame()->getMod();
	SavedGame *save = new SavedGame();

	const YAML::Node& starterBase = getGame()->getMod()->getDefaultStartingBase();
	Base& newBase = getRegistry().getService<BaseFactory>().create(mod).get<Base>();
	newBase.load(starterBase, save, true, true);

	// Kill everything we don't want in this base
	for (Soldier* soldier : newBase.getSoldiers())
	{
		delete soldier;
	}
	newBase.getSoldiers().clear();
	for (Craft* xcraft : newBase.getCrafts())
	{
		delete xcraft;
	}
	newBase.getCrafts().clear();
	newBase.getStorageItems()->clear();

	_craft = new Craft(mod.getCraft(_crafts[_cbxCraft->getSelected()]), &newBase, 1);
	newBase.getCrafts().push_back(_craft);

	// Generate soldiers
	bool psiStrengthEval = (Options::psiStrengthEval && save->isResearched(mod.getPsiRequirements()));
	for (int i = 0; i < 30; ++i)
	{
		int randomType = RNG::generate(0, (int)mod.getSoldiersList().size() - 1);
		RuleSoldier* ruleSoldier = mod.getSoldier(mod.getSoldiersList().at(randomType), true);
		int nationality = save->selectSoldierNationalityByLocation(mod, ruleSoldier, nullptr); // -1
		Soldier *soldier = mod.genSoldier(save, ruleSoldier, nationality);

		for (int n = 0; n < 5; ++n)
		{
			if (RNG::percent(70))
				continue;
			soldier->promoteRank();

			UnitStats* stats = soldier->getCurrentStatsEditable();
			stats->tu			+= RNG::generate(0, 5);
			stats->stamina		+= RNG::generate(0, 5);
			stats->health		+= RNG::generate(0, 5);
			stats->bravery		+= RNG::generate(0, 5);
			stats->reactions	+= RNG::generate(0, 5);
			stats->firing		+= RNG::generate(0, 5);
			stats->throwing		+= RNG::generate(0, 5);
			stats->strength		+= RNG::generate(0, 5);
			stats->mana			+= RNG::generate(0, 5);
			stats->psiStrength	+= RNG::generate(0, 5);
			stats->melee		+= RNG::generate(0, 5);
			stats->psiSkill		+= RNG::generate(0, 20);
		}
		UnitStats* stats = soldier->getCurrentStatsEditable();
		stats->bravery = (int)ceil(stats->bravery / 10.0) * 10; // keep it a multiple of 10

		// update again, could have been changed since soldier creation
		soldier->calcStatString(mod.getStatStrings(), psiStrengthEval);

		newBase.getSoldiers().push_back(soldier);

		int space = _craft->getSpaceAvailable();
		if (_craft->validateAddingSoldier(space, soldier) == CPE_None)
		{
			soldier->setCraft(_craft);
		}
	}

	// Generate items
	for (auto& itemType : mod.getItemsList())
	{
		const RuleItem *rule = getGame()->getMod()->getItem(itemType);
		if (rule->getBattleType() != BT_CORPSE && rule->isRecoverable())
		{
			int howMany = rule->getBattleType() == BT_AMMO ? 2 : 1;
			newBase.getStorageItems()->addItem(rule, howMany);
			if (rule->getBattleType() != BT_NONE && rule->isInventoryItem())
			{
				_craft->getItems()->addItem(rule, howMany);
			}
		}
	}

	// Add research
	for (auto& pair : mod.getResearchMap())
	{
		save->addFinishedResearchSimple(pair.second);
	}

	getGame()->setSavedGame(save);
	cbxMissionChange(0);
}

/**
 * Starts the battle.
 * @param action Pointer to an action.
 */
void NewBattleState::btnOkClick(Action *)
{
	if (_craft)
	{
		// just in case somebody manually edited battle.cfg
		_craft->resetCustomDeployment();
	}
	save();
	if (_missionTypes[_cbxMission->getSelected()] != "STR_BASE_DEFENSE" && _craft->getNumTotalUnits() == 0)
	{
		return;
	}

	SavedBattleGame *bgame = new SavedBattleGame(getGame()->getMod(), getGame()->getLanguage());
	getGame()->getSavedGame()->setBattleGame(bgame);
	bgame->setMissionType(_missionTypes[_cbxMission->getSelected()]);
	BattlescapeGenerator bgen = BattlescapeGenerator(getGame());
	Base *base = 0;

	bgen.setTerrain(getGame()->getMod()->getTerrain(_terrainTypes[_cbxTerrain->getSelected()]));

	if (_globeTextureVisible)
	{
		int textureId = _globeTextureIDs[_selectedGlobeTexture];
		auto* globeTexture = getGame()->getMod()->getGlobe()->getTexture(textureId);
		bgen.setWorldTexture(nullptr, globeTexture);
	}

	// base defense
	if (_missionTypes[_cbxMission->getSelected()] == "STR_BASE_DEFENSE")
	{
		base = _craft->getBase();
		bgen.setBase(base);
		_craft = 0;
	}
	// alien base
	else if (getGame()->getMod()->getDeployment(bgame->getMissionType())->isAlienBase())
	{
		AlienBase *b = new AlienBase(getGame()->getMod()->getDeployment(bgame->getMissionType()), -1);
		b->setId(1);
		b->setAlienRace(_alienRaces[_cbxAlienRace->getSelected()]);
		_craft->setDestination(b);
		bgen.setAlienBase(b);
		getGame()->getSavedGame()->getAlienBases().push_back(b);
	}
	// ufo assault
	else if (RuleUfo* ruleUfo = getGame()->getMod()->getUfo(_missionTypes[_cbxMission->getSelected()]); _craft)
	{
		Ufo& newUfo = getRegistry().getService<UfoFactory>().create(*ruleUfo, 1).get<Ufo>();

		_craft->setDestination(&newUfo);
		bgen.setUfo(&newUfo);
		// either ground assault or ufo crash
		if (RNG::generate(0,1) == 1)
		{
			newUfo.setStatus(Ufo::LANDED);
			bgame->setMissionType("STR_UFO_GROUND_ASSAULT");
		}
		else
		{
			newUfo.setStatus(Ufo::CRASHED);
			bgame->setMissionType("STR_UFO_CRASH_RECOVERY");
		}
	}
	// mission site
	else
	{
		const AlienDeployment *deployment = getGame()->getMod()->getDeployment(bgame->getMissionType());
		const RuleAlienMission *mission = getGame()->getMod()->getAlienMission(getGame()->getMod()->getAlienMissionList().front()); // doesn't matter
		MissionSite *m = new MissionSite(mission, deployment, nullptr);
		m->setId(1);
		m->setAlienRace(_alienRaces[_cbxAlienRace->getSelected()]);
		_craft->setDestination(m);
		bgen.setMissionSite(m);
		getGame()->getSavedGame()->getMissionSites().push_back(m);
	}

	if (_craft)
	{
		_craft->setSpeed(0);
		bgen.setCraft(_craft);
	}

	getGame()->getSavedGame()->setDifficulty((GameDifficulty)_cbxDifficulty->getSelected());

	bgen.setWorldShade(_slrDarkness->getValue());
	bgen.setAlienRace(_alienRaces[_cbxAlienRace->getSelected()]);
	bgen.setAlienItemlevel(_slrAlienTech->getValue());
	bgame->setDepth(_slrDepth->getValue());

	bgen.run();

	getGame()->popState();
	getGame()->popState();
	getGame()->pushState(new BriefingState(_craft, base));
	_craft = 0;
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void NewBattleState::btnCancelClick(Action *)
{
	if (!_surfaceBackup.empty())
	{
		cleanup();
		return;
	}

	save();
	getGame()->setSavedGame(0);
	getGame()->popState();
}

/**
 * Randomize the state
 * @param action Pointer to an action.
 */
void NewBattleState::btnRandomClick(Action *)
{
	initSave();

	_cbxMission->setSelected(RNG::generate(0, (int)_missionTypes.size()-1));
	cbxMissionChange(0);
	_cbxCraft->setSelected(RNG::generate(0, (int)_crafts.size() - 1));
	cbxCraftChange(0);
	_slrDarkness->setValue(RNG::generate(0, 15));
	_cbxTerrain->setSelected(RNG::generate(0, (int)_terrainTypes.size() - 1));
	cbxTerrainChange(0);
	_cbxAlienRace->setSelected(RNG::generate(0, (int)_alienRaces.size()-1));
	_cbxDifficulty->setSelected(RNG::generate(0, 4));
	_slrAlienTech->setValue(RNG::generate(0, (int)getGame()->getMod()->getAlienItemLevels().size()-1));
}

/**
 * Shows the Craft Info screen.
 * @param action Pointer to an action.
 */
void NewBattleState::btnEquipClick(Action *)
{
	getGame()->pushState(new CraftInfoState(getRegistry().front<Base>().try_get<Base>(), 0));
}

/**
 * Updates Map Options based on the
 * current Mission type.
 * @param action Pointer to an action.
 */
void NewBattleState::cbxMissionChange(Action *)
{
	AlienDeployment *ruleDeploy = getGame()->getMod()->getDeployment(_missionTypes[_cbxMission->getSelected()]);
	std::set<std::string> terrains;

	// Get terrains associated with this mission
	std::vector<std::string> deployTerrains, globeTerrains;
	deployTerrains = ruleDeploy->getTerrains();
	if (deployTerrains.empty())
	{
		globeTerrains = getGame()->getMod()->getGlobe()->getTerrains("");
	}
	else
	{
		globeTerrains = getGame()->getMod()->getGlobe()->getTerrains(ruleDeploy->getType());
	}
	for (const auto& terrain : deployTerrains)
	{
		terrains.insert(terrain);
	}
	for (const auto& terrain : globeTerrains)
	{
		terrains.insert(terrain);
	}
	_terrainTypes.clear();
	std::vector<std::string> terrainStrings;
	for (const auto& terrain : terrains)
	{
		_terrainTypes.push_back(terrain);
		terrainStrings.push_back("MAP_" + terrain);
	}

	// Hide controls that don't apply to mission
	_txtDarkness->setVisible(ruleDeploy->getShade() == -1);
	_slrDarkness->setVisible(ruleDeploy->getShade() == -1);
	_txtTerrain->setVisible(_terrainTypes.size() > 1);
	_cbxTerrain->setVisible(_terrainTypes.size() > 1);
	_cbxTerrain->setOptions(terrainStrings, true);
	_cbxTerrain->setSelected(0);
	_btnTerrain->setVisible(_missionTypes.size() > TFTD_DEPLOYMENTS && _terrainTypes.size() > 1);
	cbxTerrainChange(0);
}

/**
 * Updates craft accordingly.
 * @param action Pointer to an action.
 */
void NewBattleState::cbxCraftChange(Action *)
{
	_craft->changeRules(getGame()->getMod()->getCraft(_crafts[_cbxCraft->getSelected()]));

	int count = 0;
	Craft* tmpCraft = new Craft(_craft->getRules(), _craft->getBase(), 0);

	// temporarily re-assign all soldiers to a dummy craft
	for (Soldier* soldier : _craft->getBase()->getSoldiers())
	{
		if (soldier->getCraft() == _craft)
		{
			soldier->setCraft(tmpCraft);
			count++;
		}
	}
	// try assigning all soldiers back while validating constraints
	for (Soldier* soldier : _craft->getBase()->getSoldiers())
	{
		if (count <= 0)
		{
			break;
		}
		if (soldier->getCraft() == tmpCraft)
		{
			count--;
			int space = _craft->getSpaceAvailable();
			if (_craft->validateAddingSoldier(space, soldier) == CPE_None)
			{
				soldier->setCraft(_craft);
			}
			else
			{
				soldier->setCraft(0);
			}
		}
	}
	delete tmpCraft;

	// FIXME? HWPs can still violate the constraints (e.g. when switching from Avenger to Lightning)
}

/**
 * Updates the depth slider accordingly when terrain selection changes.
 * @param action Pointer to an action.
 */
void NewBattleState::cbxTerrainChange(Action *)
{
	AlienDeployment *ruleDeploy = getGame()->getMod()->getDeployment(_missionTypes[_cbxMission->getSelected()]);
	int minDepth = 0;
	int maxDepth = 0;
	if (ruleDeploy->getMaxDepth() > 0 || getGame()->getMod()->getTerrain(_terrainTypes.at(_cbxTerrain->getSelected()))->getMaxDepth() > 0 ||
		(!ruleDeploy->getTerrains().empty() && getGame()->getMod()->getTerrain(ruleDeploy->getTerrains().front())->getMaxDepth() > 0))
	{
		minDepth = 1;
		maxDepth = 3;
	}
	_depthVisible = (minDepth != maxDepth);
	_txtDepth->setVisible(_depthVisible);
	_slrDepth->setVisible(_depthVisible);
	_slrDepth->setRange(minDepth, maxDepth);
	_slrDepth->setValue(minDepth);

	{
		int found = ruleDeploy->hasTextureBasedScript(getGame()->getMod());
		if (found == -1)
		{
			// there is no map script on the alien deployment at all, perform the check on the terrain
			auto* ruleTerrain = getGame()->getMod()->getTerrain(_terrainTypes[_cbxTerrain->getSelected()]);
			found = ruleTerrain->hasTextureBasedScript(getGame()->getMod());
		}
		_globeTextureVisible = (found == 1);

		_txtGlobeTexture->setVisible(_globeTextureVisible);
		_btnGlobeTexture->setVisible(_globeTextureVisible);

		// if both are visible
		if (_globeTextureVisible && _depthVisible)
		{
			// hide the depth slider
			_txtDepth->setVisible(false);
			_slrDepth->setVisible(false);
			// and allow toggling between the texture selector and depth slider
			_btnGlobeTextureToggle->setVisible(true);
		}
		else
		{
			// not needed
			_btnGlobeTextureToggle->setVisible(false);
		}
	}

	// Get races "supported" by this mission
	_alienRaces = getGame()->getMod()->getAlienRacesList();
	int maxAlienRank = ruleDeploy->getMaxAlienRank();
	for (auto iter = _alienRaces.begin(); iter != _alienRaces.end();)
	{
		const auto& alienRace = (*iter);
		if (alienRace.find("_UNDERWATER") != std::string::npos)
		{
			iter = _alienRaces.erase(iter);
		}
		else
		{
			std::string raceName = (minDepth != maxDepth) ? alienRace + "_UNDERWATER" : alienRace;
			auto* raceRules = getGame()->getMod()->getAlienRace(raceName);
			if (!raceRules || maxAlienRank >= raceRules->getMembers())
			{
				// not enough members or race doesn't exist
				iter = _alienRaces.erase(iter);
			}
			else
			{
				++iter;
			}
		}
	}
	_cbxAlienRace->setOptions(_alienRaces, true);
}

/**
 * Shows the advanced mission selector.
 */
void NewBattleState::btnMissionChange(Action *action)
{
	fillList(NewBattleSelectType::MISSION, getGame()->isRightClick(action));
}

/**
 * Shows the advanced terrain selector.
 */
void NewBattleState::btnTerrainChange(Action *action)
{
	fillList(NewBattleSelectType::TERRAIN, getGame()->isRightClick(action));
}

/**
 * Shows the advanced globe texture selector.
 */
void NewBattleState::btnGlobeTextureChange(Action *action)
{
	fillList(NewBattleSelectType::GLOBETEXTURE, getGame()->isRightClick(action));
}

/**
 * Toggles between the globe texture selector and depth slider.
 */
void NewBattleState::btnGlobeTextureToggle(Action *action)
{
	_txtDepth->setVisible(!_txtDepth->getVisible());
	_slrDepth->setVisible(!_slrDepth->getVisible());

	_txtGlobeTexture->setVisible(!_txtGlobeTexture->getVisible());
	_btnGlobeTexture->setVisible(!_btnGlobeTexture->getVisible());
}

/**
 * Shows the advanced alien race selector.
 */
void NewBattleState::btnAlienRaceChange(Action *action)
{
	fillList(NewBattleSelectType::ALIENRACE, getGame()->isRightClick(action));
}

/**
 * Fills the advanced selector with data.
 */
void NewBattleState::fillList(NewBattleSelectType selectType, bool isRightClick)
{
	_selectType = selectType;
	_isRightClick = isRightClick;

	bool firstRun = false;
	if (_surfaceBackup.empty())
	{
		firstRun = true;
		for (entt::entity surfaceEnt : _surfaces)
		{
			Surface* surface = getRegistry().raw().get<SurfaceComponent>(surfaceEnt).getSurface();
			_surfaceBackup[surface] = surface->getVisible();
			surface->setVisible(false);
		}

		Surface* windowSurface = getRegistry().raw().get<SurfaceComponent>(_window).getSurface();
		windowSurface->setVisible(true);

		_txtTitle->setVisible(true);
		_btnCancel->setVisible(true);
		_btnRandom->setVisible(false);
		_lstSelect->setVisible(true);
	}

	std::string searchString = _btnQuickSearch->getText();
	Unicode::upperCase(searchString);

	size_t counter = 0;
	auto fill = [&](const std::vector<std::string>& list, bool prefix, size_t scroll)
	{
		for (auto& m : list)
		{
			if (!searchString.empty())
			{
				std::string itemName;
				if (_isRightClick) { itemName = m; } else { itemName = tr(prefix ? "MAP_" + m : m); }
				Unicode::upperCase(itemName);
				if (itemName.find(searchString) == std::string::npos)
				{
					counter++;
					continue;
				}
			}
			_filtered.push_back(counter);
			counter++;
			_lstSelect->addRow(1, _isRightClick ? m.c_str() : tr(prefix ? "MAP_" + m : m).c_str());
		}
		if (firstRun && _lstSelect->isScrollbarVisible()) _lstSelect->scrollTo(scroll);
	};

	_filtered.clear();
	_lstSelect->clearList();
	if (_selectType == NewBattleSelectType::MISSION)
	{
		fill(_missionTypes, false, _cbxMission->getSelected());
	}
	else if (_selectType == NewBattleSelectType::TERRAIN)
	{
		fill(_terrainTypes, true, _cbxTerrain->getSelected());
	}
	else if (_selectType == NewBattleSelectType::GLOBETEXTURE)
	{
		fill(_globeTextures, false, _selectedGlobeTexture);
	}
	else if (_selectType == NewBattleSelectType::ALIENRACE)
	{
		fill(_alienRaces, false, _cbxAlienRace->getSelected());
	}
}

/**
 * L-click: Selects an item from the advanced selector and closes the selector.
 * R-click: Displays the code of the selected item.
 * M-click: Displays the translation of the selected item.
 */
void NewBattleState::lstSelectClick(Action *action)
{
	auto selected = _lstSelect->getSelectedRow();

	// quick toggle
	if (getGame()->isRightClick(action) || getGame()->isMiddleClick(action))
	{
		auto& list =
			(_selectType == NewBattleSelectType::MISSION ? _missionTypes :
			(_selectType == NewBattleSelectType::TERRAIN ? _terrainTypes :
			(_selectType == NewBattleSelectType::GLOBETEXTURE ? _globeTextures : _alienRaces)));
		std::string s = list[_filtered[selected]];
		if (getGame()->isMiddleClick(action))
		{
			s = tr((_selectType == NewBattleSelectType::TERRAIN) ? "MAP_" + s : s);
		}
		_lstSelect->setCellText(selected, 0, s.c_str());
		return;
	}

	// restore the GUI
	cleanup();

	// select the new value
	if (_selectType == NewBattleSelectType::MISSION)
	{
		_cbxMission->setSelected(_filtered[selected]);
		cbxMissionChange(0);
	}
	else if (_selectType == NewBattleSelectType::TERRAIN)
	{
		_cbxTerrain->setSelected(_filtered[selected]);
		cbxTerrainChange(0);
	}
	else if (_selectType == NewBattleSelectType::GLOBETEXTURE)
	{
		_selectedGlobeTexture = _filtered[selected];
		_btnGlobeTexture->setText(tr(_globeTextures[_selectedGlobeTexture]));
	}
	else if (_selectType == NewBattleSelectType::ALIENRACE)
	{
		_cbxAlienRace->setSelected(_filtered[selected]);
	}
}

/**
 * Restores the GUI after closing the advanced selector.
 */
void NewBattleState::cleanup()
{
	_btnQuickSearch->setText("");
	_btnQuickSearch->setVisible(false);

	for (auto& surfacePair : _surfaceBackup)
	{
		surfacePair.first->setVisible(surfacePair.second);
	}
	_surfaceBackup.clear();
}

/**
 * Quick search toggle.
 * @param action Pointer to an action.
 */
void NewBattleState::btnQuickSearchToggle(Action *action)
{
	if (_surfaceBackup.empty())
	{
		return;
	}

	if (_btnQuickSearch->getVisible())
	{
		_btnQuickSearch->setText("");
		_btnQuickSearch->setVisible(false);
		btnQuickSearchApply(action);
	}
	else
	{
		_btnQuickSearch->setVisible(true);
		_btnQuickSearch->setFocus(true);
	}
}

/**
 * Quick search.
 * @param action Pointer to an action.
 */
void NewBattleState::btnQuickSearchApply(Action *)
{
	fillList(_selectType, _isRightClick);
}

}
