/*
 * Copyright 2010-2019 OpenXcom Developers.
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
#include "GeoscapeEventState.h"
#include <map>
#include "../Basescape/SellState.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/RNG.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Interface/TextButton.h"
#include "../Interface/ToggleTextButton.h"
#include "../Interface/Window.h"
#include "../Menu/ErrorMessageState.h"
#include "../Mod/City.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleEvent.h"
#include "../Mod/RuleInterface.h"
#include "../Mod/RuleRegion.h"
#include "../Mod/RuleSoldier.h"
#include "../Savegame/AreaSystem.h"
#include "../Savegame/Base.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/Region.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/Transfer.h"
#include "../Ufopaedia/Ufopaedia.h"
#include "../Entity/Interface/Interface.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Geoscape Event window.
 * @param geoEvent Pointer to the event.
 */
GeoscapeEventState::GeoscapeEventState(const RuleEvent& eventRule) : State("GeoscapeEventState", false), _eventRule(eventRule)
{
	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();

	// Create objects
	_window = factory.createWindow("windowName", this, 256, 176, 32, 12, WindowPopup::POPUP_BOTH);
	_txtTitle = new Text(236, 32, 42, 23);
	_txtMessage = new Text(236, 96, 42, 58);
	_btnOk = new TextButton(108, 18, 48, 158);
	_btnItemsArriving = new ToggleTextButton(108, 18, 164, 158);
	_txtItem = new Text(114, 9, 44, 58);
	_txtQuantity = new Text(94, 9, 182, 58);
	_lstTransfers = new TextList(216, 80, 42, 69);

	// Set palette
	setInterface("geoscapeEvent");

	add(_window, "window", "geoscapeEvent");
	add(_txtTitle, "text1", "geoscapeEvent");
	add(_txtMessage, "text2", "geoscapeEvent");
	add(_btnOk, "button", "geoscapeEvent");
	add(_btnItemsArriving, "button", "geoscapeEvent");
	add(_txtItem, "text2", "geoscapeEvent");
	add(_txtQuantity, "text2", "geoscapeEvent");
	add(_lstTransfers, "list", "geoscapeEvent");

	centerAllSurfaces();

	// Set up objects
	WindowSystem& windowSystem = getGame()->getECS().getSystem<WindowSystem>();
	windowSystem.setBackground(_window, getGame()->getMod()->getSurface(_eventRule.getBackground()));

	_txtTitle->setAlign(TextHAlign::ALIGN_CENTER);
	_txtTitle->setBig();
	_txtTitle->setWordWrap(true);
	_txtTitle->setText(tr(_eventRule.getName()));

	_txtMessage->setVerticalAlign(TextVAlign::ALIGN_TOP);
	_txtMessage->setWordWrap(true);
	_txtMessage->setText(tr(_eventRule.getDescription()));
	if (_eventRule.alignBottom())
	{
		_txtMessage->setVerticalAlign(TextVAlign::ALIGN_BOTTOM);
	}
	_txtMessage->setScrollable(true);

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)& GeoscapeEventState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&GeoscapeEventState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&GeoscapeEventState::btnOkClick, Options::keyCancel);

	_btnItemsArriving->setText(tr("STR_ITEMS_ARRIVING"));
	_btnItemsArriving->onMouseClick((ActionHandler)&GeoscapeEventState::btnItemsArrivingClick);

	_txtItem->setText(tr("STR_ITEM"));
	_txtQuantity->setText(tr("STR_QUANTITY_UC"));

	_lstTransfers->setColumns(2, 155, 41);
	_lstTransfers->setSelectable(true);
	_lstTransfers->setBackground(_window);
	_lstTransfers->setMargin(2);

	eventLogic();

	_txtItem->setVisible(false);
	_txtQuantity->setVisible(false);
	_lstTransfers->setVisible(false);

	if (_lstTransfers->getTexts() == 0 || !Options::oxceGeoscapeEventsInstantDelivery)
	{
		_btnOk->setX((_btnOk->getX() + _btnItemsArriving->getX()) / 2);
		_btnItemsArriving->setVisible(false);
	}
}

/**
 * Helper performing event logic.
 */
void GeoscapeEventState::eventLogic()
{
	SavedGame *save = getGame()->getSavedGame();
	Base* hq = getRegistry().front<Base>().try_get<Base>();
	const Mod *mod = getGame()->getMod();
	const RuleEvent &rule = _eventRule;

	RuleRegion *regionRule = nullptr;
	City* city = nullptr;
	if (!rule.getRegionList().empty())
	{
		size_t pickRegion = static_cast<size_t>(RNG::generate(0, static_cast<int>(rule.getRegionList().size() - 1)));
		std::string regionName = rule.getRegionList().at(pickRegion);
		regionRule = getGame()->getMod()->getRegion(regionName, true);
		std::string place = tr(regionName);

		if (rule.isCitySpecific())
		{
			size_t cities = regionRule->getCities().size();
			if (cities > 0)
			{
				size_t pickCity = static_cast<size_t>(RNG::generate(0, static_cast<int>(cities - 1)));
				city = regionRule->getCities().at(pickCity);
				place = city->getName(getGame()->getLanguage());
			}
		}

		std::string titlePlus = tr(rule.getName()).arg(place);
		_txtTitle->setText(titlePlus);

		std::string messagePlus = tr(rule.getDescription()).arg(place);
		_txtMessage->setText(messagePlus);
	}

	// even if the event isn't city-specific, we'll still pick one city randomly to represent the region (and maybe even a country)
	if (regionRule)
	{
		if (!rule.isCitySpecific())
		{
			size_t cities = regionRule->getCities().size();
			if (cities > 0)
			{
				size_t pickCity = static_cast<size_t>(RNG::generate(0, static_cast<int>(cities - 1)));
				city = regionRule->getCities().at(pickCity);
			}
		}
	}

	// 1. give/take score points
	if (regionRule)
	{
		if (Region* region = getRegistry().findValueByName<Region>(regionRule->getType()))
		{
			region->addActivityXcom(rule.getPoints());
		}
	}
	else
	{
		save->addResearchScore(rule.getPoints());
	}

	// 2. give/take funds
	save->setFunds(save->getFunds() + rule.getFunds());

	// 3. spawn/transfer persons (soldiers, engineers, scientists, ...)
	const std::string& spawnedPersonType = rule.getSpawnedPersonType();
	if (rule.getSpawnedPersons() > 0 && !spawnedPersonType.empty())
	{
		if (spawnedPersonType == "STR_SCIENTIST")
		{
			Transfer* t = new Transfer(24);
			t->setScientists(rule.getSpawnedPersons());
			hq->getTransfers().push_back(t);
		}
		else if (spawnedPersonType == "STR_ENGINEER")
		{
			Transfer* t = new Transfer(24);
			t->setEngineers(rule.getSpawnedPersons());
			hq->getTransfers().push_back(t);
		}
		else
		{
			RuleSoldier* ruleSoldier = mod->getSoldier(spawnedPersonType);
			if (ruleSoldier)
			{
				for (int i = 0; i < rule.getSpawnedPersons(); ++i)
				{
					Transfer* t = new Transfer(24);
					int nationality = getGame()->getSavedGame()->selectSoldierNationalityByLocation(*getGame()->getMod(), ruleSoldier, city);
					Soldier* s = mod->genSoldier(save, ruleSoldier, nationality);
					s->load(rule.getSpawnedSoldierTemplate(), mod, save, mod->getScriptGlobal(), true); // load from soldier template
					if (!rule.getSpawnedPersonName().empty())
					{
						s->setName(tr(rule.getSpawnedPersonName()));
					}
					else
					{
						s->genName();
					}
					t->setSoldier(s);
					hq->getTransfers().push_back(t);
				}
			}
		}
	}

	// 3. spawn/transfer item into the HQ
	std::map<std::string, int> itemsToTransfer;

	for (auto &pair : rule.getEveryMultiItemList())
	{
		const RuleItem *itemRule = mod->getItem(pair.first, true);
		if (itemRule)
		{
			itemsToTransfer[itemRule->getType()] += pair.second;
		}
	}

	for (auto &itemName : rule.getEveryItemList())
	{
		const RuleItem *itemRule = mod->getItem(itemName, true);
		if (itemRule)
		{
			itemsToTransfer[itemRule->getType()] += 1;
		}
	}

	if (!rule.getRandomItemList().empty())
	{
		size_t pickItem = RNG::generate(0, (int)rule.getRandomItemList().size() - 1);
		const RuleItem *randomItem = mod->getItem(rule.getRandomItemList().at(pickItem), true);
		if (randomItem)
		{
			itemsToTransfer[randomItem->getType()] += 1;
		}
	}

	if (!rule.getRandomMultiItemList().empty())
	{
		size_t pickItem = RNG::generate(0, (int)rule.getRandomMultiItemList().size() - 1);
		auto& sublist = rule.getRandomMultiItemList().at(pickItem);
		for (auto& pair : sublist)
		{
			const RuleItem* itemRule = mod->getItem(pair.first, true);
			if (itemRule)
			{
				itemsToTransfer[itemRule->getType()] += pair.second;
			}
		}
	}

	if (!rule.getWeightedItemList().empty())
	{
		const RuleItem *randomItem = mod->getItem(rule.getWeightedItemList().choose(), true);
		if (randomItem)
		{
			itemsToTransfer[randomItem->getType()] += 1;
		}
	}

	for (auto &ti : itemsToTransfer)
	{
		if (Options::oxceGeoscapeEventsInstantDelivery)
		{
			hq->getStorageItems()->addItem(mod->getItem(ti.first, true), ti.second);
		}
		else
		{
			Transfer* t = new Transfer(1);
			t->setItems(mod->getItem(ti.first, true), ti.second);
			hq->getTransfers().push_back(t);
		}

		std::ostringstream ss;
		ss << ti.second;
		_lstTransfers->addRow(2, tr(ti.first).c_str(), ss.str().c_str());
	}

	// 3b. spawn craft into the HQ
	const RuleCraft* craftRule = mod->getCraft(rule.getSpawnedCraftType(), true);
	if (craftRule)
	{
		Craft* craft = new Craft(craftRule, hq, save->getId(craftRule->getType()));
		craft->initFixedWeapons(mod);
		if (Options::oxceGeoscapeEventsInstantDelivery)
		{
			// same as manufacture
			craft->checkup();
			hq->getCrafts().push_back(craft);
		}
		else
		{
			// same as buy
			craft->setStatus("STR_REFUELLING");
			Transfer* t = new Transfer(1);
			t->setCraft(craft);
			hq->getTransfers().push_back(t);
		}
	}

	// 4. give bonus research
	std::vector<const RuleResearch*> possibilities;

	for (auto rName : rule.getResearchList())
	{
		const RuleResearch *rRule = mod->getResearch(rName, true);
		if (!save->isResearched(rRule, false) || save->hasUndiscoveredGetOneFree(rRule, true))
		{
			possibilities.push_back(rRule);
		}
	}

	std::vector<const RuleResearch*> topicsToCheck;
	if (!possibilities.empty())
	{
		size_t pickResearch = RNG::generate(0, (int)possibilities.size() - 1);
		const RuleResearch *eventResearch = possibilities.at(pickResearch);

		bool alreadyResearched = false;
		std::string name = eventResearch->getLookup().empty() ? eventResearch->getName() : eventResearch->getLookup();
		if (save->isResearched(name, false))
		{
			alreadyResearched = true; // we have seen the pedia article already, don't show it again
		}

		save->addFinishedResearch(eventResearch, mod, hq, true);
		topicsToCheck.push_back(eventResearch);
		_researchName = alreadyResearched ? "" : eventResearch->getName();

		if (!eventResearch->getLookup().empty())
		{
			const RuleResearch *lookupResearch = mod->getResearch(eventResearch->getLookup(), true);
			save->addFinishedResearch(lookupResearch, mod, hq, true);
			_researchName = alreadyResearched ? "" : lookupResearch->getName();
		}

		if (auto bonus = save->selectGetOneFree(eventResearch))
		{
			save->addFinishedResearch(bonus, mod, hq, true);
			topicsToCheck.push_back(bonus);
			_bonusResearchName = bonus->getName();

			if (!bonus->getLookup().empty())
			{
				const RuleResearch *bonusLookup = mod->getResearch(bonus->getLookup(), true);
				save->addFinishedResearch(bonusLookup, mod, hq, true);
				_bonusResearchName = bonusLookup->getName();
			}
		}
	}

	// Side effects:
	// 1. remove obsolete research projects from all bases
	// 2. handle items spawned by research
	// 3. handle events spawned by research
	save->handlePrimaryResearchSideEffects(topicsToCheck, mod, hq);

	if (Options::oxceGeoscapeDebugLogMaxEntries > 0)
	{
		std::ostringstream ss;
		ss << "gameTime: " << save->getTime()->getFullString();
		ss << " eventPopup: " << rule.getName();
		save->getGeoscapeDebugLog().push_back(ss.str());
	}
}

/**
 *
 */
GeoscapeEventState::~GeoscapeEventState()
{
	// Empty by design
}

/**
 * Initializes the state.
 */
void GeoscapeEventState::init()
{
	State::init();

	if (!_eventRule.getMusic().empty())
	{
		getGame()->getMod()->playMusic(_eventRule.getMusic());
	}
}

/**
 * Closes the window and shows a pedia article if needed.
 * @param action Pointer to an action.
 */
void GeoscapeEventState::btnOkClick(Action *)
{
	getGame()->popState();

	if (entt::handle baseHandle = getRegistry().front<Base>();
		getGame()->getSavedGame()->getMonthsPassed() > -1 && Options::storageLimitsEnforced && baseHandle.get<Base>().storesOverfull())
	{
		getGame()->pushState(new SellState(&baseHandle.get<Base>(), nullptr));
		getGame()->pushState(new ErrorMessageState(tr("STR_STORAGE_EXCEEDED").arg(baseHandle.get<Base>().getName()), _palette,
			getGame()->getMod()->getInterface("debriefing")->getElement("errorMessage")->color, "BACK01.SCR",
			getGame()->getMod()->getInterface("debriefing")->getElement("errorPalette")->color));
	}

	if (!_bonusResearchName.empty())
	{
		Ufopaedia::openArticle(getGame(), _bonusResearchName);
	}
	if (!_researchName.empty())
	{
		Ufopaedia::openArticle(getGame(), _researchName);
	}
}

/**
 * Toggles the view between the description and the ItemsArriving list.
 * @param action Pointer to an action.
 */
void GeoscapeEventState::btnItemsArrivingClick(Action *)
{
	if (_btnItemsArriving->getPressed())
	{
		_txtMessage->setVisible(false);

		_txtItem->setVisible(true);
		_txtQuantity->setVisible(true);
		_lstTransfers->setVisible(true);
	}
	else
	{
		_txtItem->setVisible(false);
		_txtQuantity->setVisible(false);
		_lstTransfers->setVisible(false);

		_txtMessage->setVisible(true);
	}
}

}
