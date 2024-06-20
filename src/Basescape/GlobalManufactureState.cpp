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
#include "GlobalManufactureState.h"
#include <sstream>
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Unicode.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/Base.h"
#include "../Savegame/SavedGame.h"
#include "../Mod/RuleManufacture.h"
#include "../Savegame/Production.h"
#include "ManufactureState.h"
#include "TechTreeViewerState.h"
#include "../Ufopaedia/Ufopaedia.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the GlobalManufacture screen.
 */
GlobalManufactureState::GlobalManufactureState(bool openedFromBasescape) : State("GlobalManufactureState", true), _openedFromBasescape(openedFromBasescape)
{
	InterfaceFactory& factory = getGame()->getInterfaceFactory();

	// Create objects
	_window = factory.createWindow("globalManufactureMenu", this, 320, 200, 0, 0);
	_btnOk = new TextButton(304, 16, 8, 176);
	_txtTitle = new Text(310, 17, 5, 8);
	_txtAvailable = new Text(150, 9, 8, 24);
	_txtAllocated = new Text(150, 9, 160, 24);
	_txtSpace = new Text(150, 9, 8, 34);
	_txtFunds = new Text(150, 9, 160, 34);
	_txtItem = new Text(80, 9, 10, 52);
	_txtEngineers = new Text(56, 18, 112, 44);
	_txtProduced = new Text(56, 18, 168, 44);
	_txtCost = new Text(44, 27, 222, 44);
	_txtTimeLeft = new Text(60, 27, 260, 44);
	_lstManufacture = new TextList(288, 88, 8, 80);

	// Set palette
	setInterface("globalManufactureMenu");

	add(_window, "window", "globalManufactureMenu");
	add(_btnOk, "button", "globalManufactureMenu");
	add(_txtTitle, "text1", "globalManufactureMenu");
	add(_txtAvailable, "text1", "globalManufactureMenu");
	add(_txtAllocated, "text1", "globalManufactureMenu");
	add(_txtSpace, "text1", "globalManufactureMenu");
	add(_txtFunds, "text1", "globalManufactureMenu");
	add(_txtItem, "text2", "globalManufactureMenu");
	add(_txtEngineers, "text2", "globalManufactureMenu");
	add(_txtProduced, "text2", "globalManufactureMenu");
	add(_txtCost, "text2", "globalManufactureMenu");
	add(_txtTimeLeft, "text2", "globalManufactureMenu");
	add(_lstManufacture, "list", "globalManufactureMenu");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "globalManufactureMenu");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&GlobalManufactureState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&GlobalManufactureState::btnOkClick, Options::keyCancel);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_PRODUCTION_OVERVIEW"));

	_txtFunds->setText(tr("STR_CURRENT_FUNDS").arg(Unicode::formatFunding(getGame()->getSavedGame()->getFunds())));

	_txtItem->setText(tr("STR_ITEM"));

	_txtEngineers->setText(tr("STR_ENGINEERS__ALLOCATED"));
	_txtEngineers->setWordWrap(true);

	_txtProduced->setText(tr("STR_UNITS_PRODUCED"));
	_txtProduced->setWordWrap(true);

	_txtCost->setText(tr("STR_COST__PER__UNIT"));
	_txtCost->setWordWrap(true);

	_txtTimeLeft->setText(tr("STR_DAYS_HOURS_LEFT"));
	_txtTimeLeft->setWordWrap(true);

	_lstManufacture->setColumns(5, 114, 16, 52, 56, 48);
	_lstManufacture->setAlign(ALIGN_RIGHT);
	_lstManufacture->setAlign(ALIGN_LEFT, 0);
	_lstManufacture->setSelectable(true);
	_lstManufacture->setBackground(_window);

	_lstManufacture->setMargin(2);
	_lstManufacture->setWordWrap(true);
	_lstManufacture->onMouseClick((ActionHandler)&GlobalManufactureState::onSelectBase, SDL_BUTTON_LEFT);
	_lstManufacture->onMouseClick((ActionHandler)&GlobalManufactureState::onOpenTechTreeViewer, SDL_BUTTON_MIDDLE);
}

/**
 *
 */
GlobalManufactureState::~GlobalManufactureState()
{

}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void GlobalManufactureState::btnOkClick(Action *)
{
	getGame()->popState();
}

/**
 * Goes to the base's manufacture screen when clicking its project
 * @param action Pointer to an action.
 */
void GlobalManufactureState::onSelectBase(Action *)
{
	entt::entity baseId = _baseIds[_lstManufacture->getSelectedRow()];
	if (Base* base = getRegistry().raw().try_get<Base>(baseId))
	{
		// close this window
		getGame()->popState();

		// close Manufacture UI (goes back to BaseView)
		if (_openedFromBasescape)
		{
			getGame()->popState();
		}

		// open new window
		getGame()->pushState(new ManufactureState(base));
	}
}

/**
 * Opens the TechTreeViewer for the corresponding topic.
 * @param action Pointer to an action.
 */
void GlobalManufactureState::onOpenTechTreeViewer(Action *)
{
	const RuleManufacture *selectedTopic = _topics[_lstManufacture->getSelectedRow()];

	if (selectedTopic)
	{
		if (getGame()->isCtrlPressed())
		{
			std::string articleId = selectedTopic->getName();
			Ufopaedia::openArticle(getGame(), articleId);
		}
		else
		{
			getGame()->pushState(new TechTreeViewerState(0, selectedTopic));
		}
	}
}

/**
 * Updates the production list
 * after going to other screens.
 */
void GlobalManufactureState::init()
{
	State::init();
	fillProductionList();
}

/**
 * Fills the list with Productions from all bases. Also updates total count of available workshop space and available/allocated engineers.
 */
void GlobalManufactureState::fillProductionList()
{
	_baseIds.clear();
	_topics.clear();
	_lstManufacture->clearList();

	int availableEngineers = 0;
	int allocatedEngineers = 0;
	int freeWorkshops = 0;

	for (auto&& [ id, xcomBase ] : getRegistry().raw().view<Base>().each())
	{
		auto& baseProductions = xcomBase.getProductions();
		if (!baseProductions.empty() || xcomBase.getEngineers() > 0)
		{
			std::string baseName = xcomBase.getName(getGame()->getLanguage());
			_lstManufacture->addRow(3, baseName.c_str(), "", "");
			_lstManufacture->setRowColor(_lstManufacture->getLastRowIndex(), _lstManufacture->getSecondaryColor());

			// dummy
			_baseIds.push_back(entt::null);
			_topics.push_back(0);
		}
		for (const auto* prod : baseProductions)
		{
			std::string producedOfTotal = std::format("{}/{}{}", prod->getAmountProduced(),
				prod->getInfiniteAmount() ? "∞" : std::to_string(prod->getAmountTotal()),
				prod->getSellItems() ? " $" : "");

			std::string timeLeft;
			if (prod->getInfiniteAmount()) { timeLeft = "∞"; }
			else if (prod->getAssignedEngineers() > 0)
			{
				int hoursLeft = prod->getAmountTotal() * prod->getRules()->getManufactureTime() - prod->getTimeSpent();
				int numEffectiveEngineers = prod->getAssignedEngineers();
				// ensure we round up since it takes an entire hour to manufacture any part of that hour's capacity
				hoursLeft = (hoursLeft + numEffectiveEngineers - 1) / numEffectiveEngineers;
				int daysLeft = hoursLeft / 24;
				int hours = hoursLeft % 24;
				timeLeft = std::format("{}/{}", daysLeft, hours);
			}
			else { timeLeft = "-"; }
			_lstManufacture->addRow(5,
				tr(prod->getRules()->getName()).c_str(),
				std::to_string(prod->getAssignedEngineers()), producedOfTotal,
				Unicode::formatFunding(prod->getRules()->getManufactureCost()),
				timeLeft);

			_baseIds.push_back(id);
			_topics.push_back(prod->getRules());
		}
		if (baseProductions.empty() && xcomBase.getEngineers() > 0)
		{
			_lstManufacture->addRow(5, tr("STR_NONE").c_str(), "", "", "", "");

			_baseIds.push_back(id);
			_topics.push_back(0);
		}

		availableEngineers += xcomBase.getAvailableEngineers();
		allocatedEngineers += xcomBase.getAllocatedEngineers();
		freeWorkshops += xcomBase.getFreeWorkshops();
	}

	_txtAvailable->setText(tr("STR_ENGINEERS_AVAILABLE").arg(availableEngineers));
	_txtAllocated->setText(tr("STR_ENGINEERS_ALLOCATED").arg(allocatedEngineers));
	_txtSpace->setText(tr("STR_WORKSHOP_SPACE_AVAILABLE").arg(freeWorkshops));
}

}
