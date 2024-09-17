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
#include "SoldiersAIState.h"
#include <algorithm>
#include <climits>
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Engine/Unicode.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/ComboBox.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Savegame/Base.h"
#include "../Savegame/BattleUnit.h"
#include "../Savegame/Craft.h"
#include "../Savegame/Soldier.h"
#include "../Entity/Interface/Interface.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Soldiers AI screen.
 * @param soldiers Soldiers to manipulate AI control
 */
SoldiersAIState::SoldiersAIState(std::vector<Soldier*>& soldiers)
	: State("SoldiersAIState", true), _soldiers(soldiers)
{
	_commonConstruct();
}

/**
 * Initializes all the elements in the Soldiers AI screen.
 * @param craft Craft whose soldiers to manipulate AI control
 */
SoldiersAIState::SoldiersAIState(const Craft* craft)
	: State("SoldiersAIState", true)
{
	for (Soldier* soldier : craft->getBase()->getSoldiers())
	{
		if (soldier->getCraft() == craft)
			_soldiers.push_back(soldier);
	}
	_commonConstruct();
}

/**
 * Initializes all the elements in the Soldiers AI screen.
 * @param soldiers Soldiers to manipulate AI control
 */
SoldiersAIState::SoldiersAIState(std::vector<BattleUnit*>& units)
	: State("SoldiersAIState", true), _units(units)
{
	_commonConstruct();
}

/**
 * Initializes all the elements in the Soldiers AI screen.
 */
void SoldiersAIState::_commonConstruct()
{
	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();

	// Create objects
	_window = factory.createWindow("craftSoldiers", this, 320, 200, 0, 0);
	_btnOk = new TextButton(148, 16, 164, 176);
	_txtTitle = new Text(300, 17, 16, 7);
	_lstUnits = new TextList(288, 128, 8, 40);
	// list column headers
	constexpr int gap = 4; //TODO adjust?
	int xoff = 16;
	static constexpr int cWidths[] = {110, 60, 55, 30};	//TODO adjust?
	_txtName = new Text(cWidths[0], 9, xoff, 32);
	xoff += cWidths[0] + gap;
	_txtRank = new Text(cWidths[1], 9, xoff, 32);
	xoff += cWidths[1] + gap;
	_txtControlled = new Text(cWidths[2], 9, xoff, 32);
	xoff += cWidths[2] + gap;
	_txtAgressiveness = new Text(cWidths[3], 9, xoff, 32);

	// Set palette
	setInterface("craftSoldiers");

	add(_window, "window", "craftSoldiers");
	add(_btnOk, "button", "craftSoldiers");
	add(_txtTitle, "text", "craftSoldiers");
	add(_txtName, "text", "craftSoldiers");
	add(_txtRank, "text", "craftSoldiers");
	add(_txtControlled, "text", "craftSoldiers");
	add(_txtAgressiveness, "text", "craftSoldiers");
	add(_lstUnits, "list", "craftSoldiers");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "craftSoldiers");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&SoldiersAIState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&SoldiersAIState::btnOkClick, Options::keyCancel);

	_txtTitle->setBig();
	_txtTitle->setText(tr("STR_SELECT_SOLDIERS_AI"));

	_txtName->setText(tr("STR_NAME_UC"));

	_txtRank->setText(tr("STR_RANK"));

	_txtControlled->setText(tr("STR_AI_CONTROLLED"));
	_txtAgressiveness->setText(tr("STR_PER_UNIT_AGGRESSION"));		//TODO different name / text?

	//_lstUnits->setArrowColumn(188, ARROW_VERTICAL);	//Input mostly temporary vector, so reordering not persistent. Disable completly
	_lstUnits->setColumns(noCol, cWidths[0], cWidths[1], cWidths[2], cWidths[3]);	//TODO with or without gap?
	_lstUnits->setAlign(TextHAlign::ALIGN_RIGHT, 3);
	_lstUnits->setSelectable(true);
	_lstUnits->setBackground(_window);

	_lstUnits->setMargin(8);
	_lstUnits->onMouseClick((ActionHandler)&SoldiersAIState::lstSoldiersClick, 0);
}

/**
 * cleans up dynamic state
 */
SoldiersAIState::~SoldiersAIState()
{
}


/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void SoldiersAIState::btnOkClick(Action *)
{
	getGame()->popState();
}

/**
 * Shows the units/soldiers in a list at specified offset/scroll.
 */
void SoldiersAIState::initList(size_t scrl)
{
	_lstUnits->clearList();

	std::vector<bool> allows;
	if (!_soldiers.empty())
	{
		for (const auto* s : _soldiers)
		{
			_lstUnits->addRow(noCol, s->getName(true, 19).c_str(), tr(s->getRankString()).c_str(), "", std::to_string(s->getAggression()).c_str());
			allows.emplace_back(s->getAllowAutoCombat());
		}
	}
	else
	{
		for (const auto* u : _units)
		{
			const std::string name = u->getGeoscapeSoldier() ? u->getGeoscapeSoldier()->getName(true, 19) : u->getName(getGame()->getLanguage());	//BattleUnit::getName has no maxLength parameter. Default value might change and Statstring might be way to long.
			const std::string rank = u->getRankString();
			_lstUnits->addRow(noCol, name.c_str(), tr(rank).c_str(), "", std::to_string(u->getAggression()).c_str());
			allows.emplace_back(u->getAllowAutoCombat());
		}
	}
	
	for (int row = 0; row < allows.size(); row++)
	{
		uint8_t color;
		if (allows[row])
		{
			color = _lstUnits->getSecondaryColor();
			_lstUnits->setCellText(row, 2, tr("True"));
		}
		else
		{
			color = _lstUnits->getColor();
			_lstUnits->setCellText(row, 2, tr("False"));
		}
		_lstUnits->setRowColor(row, color);
	}
	
	if (scrl)
		_lstUnits->scrollTo(scrl);
	_lstUnits->draw();
}

/**
 * Shows the units/soldiers in a list.
 */
void SoldiersAIState::init()
{
	State::init();
	initList(0);
}

/**
 * Toggle the alllowAutoCombat flag for selected unit/soldier
 * @param action Pointer to an action.
 */
void SoldiersAIState::lstSoldiersClick(Action *action)
{
	double mx = action->getAbsoluteXMouse();
	if (mx >= _lstUnits->getArrowsLeftEdge() && mx < _lstUnits->getArrowsRightEdge())
	{
		return;
	}
	int row = _lstUnits->getSelectedRow();
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		const bool newAI = _soldiers.empty() ? toggleAIBattleUnit() : toggleAISoldier();
		uint8_t color = _lstUnits->getColor();
		if (newAI)
		{
			color = _lstUnits->getSecondaryColor();
			_lstUnits->setCellText(row, 2, tr("True"));
		}
		else
		{
			_lstUnits->setCellText(row, 2, tr("False"));
		}
		_lstUnits->setRowColor(row, color);
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		//Depending on future plans for the window and the AI better alternatives are
		//common parent class for soliers and battleunits
		//templating the window
		//converting the input battleunit/soldiers on the fly at construction and writing back changes at deconstruction
		//wrapper class for soldiers and battleunits
		const auto newAG = _soldiers.empty() ? toggleAgg(_units.at(row)) : toggleAgg(_soldiers.at(row));
		_lstUnits->setCellText(row, 3, std::to_string(newAG).c_str());		//TODO OXC replacement for to_string?	//TODO tr()?	//TODO look at how vanilla oxce states do that
	}
}


bool SoldiersAIState::toggleAISoldier()
{
	Soldier *s = _soldiers.at(_lstUnits->getSelectedRow());
	return s->toggleAllowAutoCombat();
}

bool SoldiersAIState::toggleAIBattleUnit()
{
	auto* bu = _units.at(_lstUnits->getSelectedRow());
	return bu->toggleAllowAutoCombat();
}

}
