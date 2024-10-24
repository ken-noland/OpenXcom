/*
 * Copyright 2010-2023 OpenXcom Developers.
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
#include "GlobalAlienContainmentState.h"
#include "ManageAlienContainmentState.h"
#include "TechTreeViewerState.h"
#include <sstream>
#include <set>
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/Registry.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleResearch.h"
#include "../Savegame/Base.h"
#include "../Savegame/BaseFacility.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/ResearchProject.h"
#include "../Savegame/SavedGame.h"
#include "../Entity/Interface/Interface.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the GlobalAlienContainment screen.
 */
GlobalAlienContainmentState::GlobalAlienContainmentState(bool openedFromBasescape) : State("GlobalAlienContainmentState", true), _openedFromBasescape(openedFromBasescape)
{
	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();

	// Create objects
	_window = factory.createWindow("globalContainmentMenu", this, 320, 200, 0, 0);
	_btnOk = new TextButton(304, 16, 8, 176);
	_txtTitle = new Text(310, 17, 5, 8);
	_txtTotalUsed = new Text(150, 9, 10, 24);
	_txtTotalInterrogated = new Text(150, 9, 130, 24);
	_txtPrisoner = new Text(146, 9, 10, 34);
	_txtPrisonerAmount = new Text(60, 9, 156, 34);
	_txtPrisonersInterrogated = new Text(80, 9, 216, 34);
	_lstPrisoners = new TextList(288, 128, 8, 44);

	// Set palette
	setInterface("globalContainmentMenu");

	add(_window, "window", "globalContainmentMenu");
	add(_btnOk, "button", "globalContainmentMenu");
	add(_txtTitle, "text", "globalContainmentMenu");
	add(_txtTotalUsed, "text", "globalContainmentMenu");
	add(_txtTotalInterrogated, "text", "globalContainmentMenu");
	add(_txtPrisoner, "text", "globalContainmentMenu");
	add(_txtPrisonerAmount, "text", "globalContainmentMenu");
	add(_txtPrisonersInterrogated, "text", "globalContainmentMenu");
	add(_lstPrisoners, "list", "globalContainmentMenu");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "globalContainmentMenu");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&GlobalAlienContainmentState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&GlobalAlienContainmentState::btnOkClick, Options::keyCancel);

	_txtTitle->setBig();
	_txtTitle->setAlign(TextHAlign::ALIGN_CENTER);
	_txtTitle->setText(tr("STR_PRISONER_OVERVIEW"));

	_txtPrisoner->setText(tr("STR_PRISONER"));

	_txtPrisonerAmount->setAlign(TextHAlign::ALIGN_CENTER);
	_txtPrisonerAmount->setText(tr("STR_PRISONER_AMOUNT"));

	_txtPrisonersInterrogated->setAlign(TextHAlign::ALIGN_CENTER);
	_txtPrisonersInterrogated->setText(tr("STR_PRISONER_INTERROGATED"));


	_lstPrisoners->setColumns(3, 146, 60, 80);
	_lstPrisoners->setSelectable(true);
	_lstPrisoners->setBackground(_window);

	_lstPrisoners->setMargin(2);
	_lstPrisoners->setWordWrap(true);
	_lstPrisoners->setAlign(TextHAlign::ALIGN_CENTER, 1);
	_lstPrisoners->setAlign(TextHAlign::ALIGN_CENTER, 2);
	_lstPrisoners->onMouseClick((ActionHandler)&GlobalAlienContainmentState::onSelectBase, SDL_BUTTON_LEFT);
	_lstPrisoners->onMouseClick((ActionHandler)&GlobalAlienContainmentState::onOpenTechTreeViewer, SDL_BUTTON_MIDDLE);
}

/**
 *
 */
GlobalAlienContainmentState::~GlobalAlienContainmentState()
{
}

/**
 * Updates the prisoner list
 * after going to other screens.
 */
void GlobalAlienContainmentState::init()
{
	State::init();
	fillPrisonerList();
}

/**
 * Fills the list with all prisoners from all bases. Also updates totals.
 */
void GlobalAlienContainmentState::fillPrisonerList()
{
	_topics.clear();
	_lstPrisoners->clearList();

	int totalInterrogated = 0;
	int totalUsed = 0;

	// determine prison types used in the game
	std::set<int> prisonTypes = { 0 };
	bool noTypes = true;
	for (const std::string& facType : getGame()->getMod()->getBaseFacilitiesList())
	{
		RuleBaseFacility* facRule = getGame()->getMod()->getBaseFacility(facType);
		if (facRule->getPrisonType() > 0)
		{
			prisonTypes.insert(facRule->getPrisonType());
			noTypes = false;
		}
	}

	for (Base& xcomBase : getRegistry().list<Base>())
	{
		bool displayed = false;
		int totalBaseCapacity = 0;

		// determine prison types used in the base
		std::set<int> occupiedPrisonTypes;
		for(int prisonType : prisonTypes)
		{
			totalBaseCapacity += xcomBase.getAvailableContainment(prisonType);

			for (BaseFacility* baseFacility : xcomBase.getFacilities())
			{
				if(baseFacility->getRules()->getAliens() > 0 && baseFacility->getRules()->getPrisonType() == prisonType)
				{
					int usedSpace = xcomBase.getUsedContainment(prisonType);
					if (usedSpace > 0)
					{
						occupiedPrisonTypes.insert(prisonType);
					}
					totalUsed += usedSpace;
					break;
				}

			}
		}

		for (int prisonType : occupiedPrisonTypes)
		{
			std::vector<std::string> researchList;
			for (const auto* proj : xcomBase.getResearch())
			{
				const RuleResearch* research = proj->getRules();
				const RuleItem* item = getGame()->getMod()->getItem(research->getName()); // don't use getNeededItem()
				if (research->needItem() && research->destroyItem() && item && item->isAlien() && item->getPrisonType() == prisonType)
				{
					researchList.push_back(research->getName());
				}
			}

			std::string baseNameAndPrisonType = xcomBase.getName(getGame()->getLanguage());
			if (!noTypes)
			{
				baseNameAndPrisonType = baseNameAndPrisonType + " - " + std::string(trAlt("STR_PRISON_TYPE", prisonType));
			}
			_lstPrisoners->addRow(3, baseNameAndPrisonType.c_str(), "", "");
			_lstPrisoners->setRowColor(_lstPrisoners->getLastRowIndex(), _lstPrisoners->getSecondaryColor());
			_topics.push_back(std::make_tuple("", nullptr, 0));
			displayed = true;

			for (auto& itemType : getGame()->getMod()->getItemsList())
			{
				RuleItem* rule = getGame()->getMod()->getItem(itemType, true);
				if (rule->isAlien() && rule->getPrisonType() == prisonType)
				{
					int qty = xcomBase.getStorageItems()->getItem(rule);
					if (qty > 0)
					{
						std::ostringstream ss;
						ss << qty;
						std::string rqty;
						auto researchIt = std::find(researchList.begin(), researchList.end(), itemType);
						if (researchIt != researchList.end())
						{
							rqty = "1";
							researchList.erase(researchIt);
							totalInterrogated++;
						}
						else
						{
							rqty = "0";
						}

						_lstPrisoners->addRow(3, tr(itemType).c_str(), ss.str().c_str(), rqty.c_str());
						_topics.push_back(std::make_tuple(itemType, &xcomBase, prisonType));
					}
				}
			}

			for (const auto& researchName : researchList)
			{
				_lstPrisoners->addRow(3, tr(researchName).c_str(), "0", "1");
				_topics.push_back(std::make_tuple(researchName, &xcomBase, prisonType));
				totalInterrogated++;
			}
		}

		if (!displayed && totalBaseCapacity > 0)
		{
			_lstPrisoners->addRow(3, xcomBase.getName(getGame()->getLanguage()).c_str(), "", "");
			_lstPrisoners->setRowColor(_lstPrisoners->getLastRowIndex(), _lstPrisoners->getSecondaryColor());
			_topics.push_back(std::make_tuple("", nullptr, 0));

			_lstPrisoners->addRow(3, tr("STR_NONE").c_str(), "", "");
			_topics.push_back(std::make_tuple("", &xcomBase, 0));
		}
	}

	_txtTotalUsed->setText(tr("STR_TOTAL_IN_PRISON").arg(totalUsed));
	_txtTotalInterrogated->setText(tr("STR_TOTAL_INTERROGATED").arg(totalInterrogated));
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void GlobalAlienContainmentState::btnOkClick(Action*)
{
	getGame()->popState();
}

/**
 * Goes to the base's research screen when clicking its project
 * @param action Pointer to an action.
 */
void GlobalAlienContainmentState::onSelectBase(Action*)
{
	auto& tuple = _topics[_lstPrisoners->getSelectedRow()];
	Base* base = std::get<1>(tuple);
	int prisonType = std::get<2>(tuple);

	if (base)
	{
		// close this window
		getGame()->popState();

		// close Manage Alien Containment UI (goes back to BaseView)
		if (_openedFromBasescape)
		{
			getGame()->popState();
		}

		// open new window
		getGame()->pushState(new ManageAlienContainmentState(base, prisonType, OPT_GEOSCAPE));
	}
}

/**
 * Opens the TechTreeViewer for the corresponding topic.
 * @param action Pointer to an action.
 */
void GlobalAlienContainmentState::onOpenTechTreeViewer(Action*)
{
	auto& tuple = _topics[_lstPrisoners->getSelectedRow()];
	const RuleResearch* selectedTopic = getGame()->getMod()->getResearch(std::get<0>(tuple));

	if (selectedTopic)
	{
		getGame()->pushState(new TechTreeViewerState(selectedTopic, 0));
	}
}

}
