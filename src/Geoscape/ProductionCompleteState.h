#pragma once
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
#include <string>
#include <map>
#include <vector>
#include "../Engine/State.h"
#include "../Savegame/Production.h"

namespace OpenXcom
{

class TextButton;

class Text;
class TextList;
class Base;
class GeoscapeState;

/**
 * Window used to notify the player when
 * a production is completed.
 */
class ProductionCompleteState : public State
{
private:
	Base *_base;
	GeoscapeState *_state;

	std::map<std::string, int> _randomProductionInfo;
	std::vector<std::string> _index;

	TextButton *_btnOk, *_btnGotoBase, *_btnSummary;
	entt::handle _window;
	Text *_txtMessage, *_txtItem, *_txtQuantity;
	TextList *_lstSummary;
	productionProgress_e _endType;
public:
	/// Creates the Production Complete state.
	ProductionCompleteState(Base *base, const std::string &item, GeoscapeState *state, productionProgress_e endType = PROGRESS_COMPLETE, Production *production = nullptr);
	/// Cleans up the Production Complete state.
	~ProductionCompleteState();
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Go To Base button.
	void btnGotoBaseClick(Action *action);
	/// Handler for clicking the Summary button.
	void btnSummaryClick(Action *action);
	/// Handler for clicking the Summary list.
	void lstSummaryClick(Action *action);
};

}
