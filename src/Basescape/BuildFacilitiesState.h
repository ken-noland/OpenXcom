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
#include <vector>
#include <entt/entt.hpp>
#include "../Engine/State.h"

namespace OpenXcom
{

class Base;
class TextButton;
class Text;
class TextList;
class RuleBaseFacility;

/**
 * Window shown with all the facilities
 * available to build.
 */
class BuildFacilitiesState : public State
{
protected:
	entt::handle _baseHandle;
	State *_state;
	std::vector<RuleBaseFacility*> _facilities, _disabledFacilities;
	size_t _lstScroll;

	TextButton *_btnOk;
	entt::handle _window;
	Text *_txtTitle;
	TextList *_lstFacilities;
public:
	/// Creates the Build Facilities state.
	BuildFacilitiesState(entt::handle baseHandle, State *state);
	/// Populates the build option list.
	virtual void populateBuildList();
	/// Updates the base stats.
	void init() override;
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Facilities list.
	virtual void lstFacilitiesClick(Action *action);
};

}
