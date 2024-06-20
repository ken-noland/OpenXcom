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
#include "../Engine/State.h"
#include <unordered_map>
#include <entt/entt.hpp>

namespace OpenXcom
{

class TextButton;
class Window;
class Text;
class TextList;
class GeoscapeState;
class SavedGame;

/**
 * Displays hidden alien activity info.
 */
class HiddenAlienActivityState : public State
{
private:
	GeoscapeState* _state;
	SavedGame& _save;

	std::unordered_map<entt::entity, int> _displayHiddenAlienActivityRegions;
	std::unordered_map<entt::entity, int> _displayHiddenAlienActivityCountries;

	TextButton* _btnOk;
	TextButton* _btnCancel;
	Window *_window;
	Text *_txtInfo, *_txtHeaderRegions, *_txtSightingsRegions, *_txtHeaderCountries, *_txtSightingsCountries;
	TextList *_lstHiddenAlienActivityRegions, *_lstHiddenAlienActivityCountries;

public:
	HiddenAlienActivityState(GeoscapeState* state, SavedGame& save,
		std::unordered_map<entt::entity, int> displayHiddenAlienActivityRegions,
		std::unordered_map<entt::entity, int> displayHiddenAlienActivityCountries);
	void btnOkClick(Action* action);
	void btnCancelClick(Action* action);
};

}
