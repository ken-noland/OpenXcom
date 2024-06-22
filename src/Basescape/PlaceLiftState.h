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
#include <entt/entt.hpp>

namespace OpenXcom
{

class Base;
class BaseView;
class MiniBaseView;
class Text;
class TextList;
class Globe;
class RuleBaseFacility;

/**
 * Screen shown when the player has to
 * place the access lift of a base.
 */
class PlaceLiftState : public State
{
private:
	entt::handle _newBaseHandle;
	Globe *_globe;
	BaseView *_view;
	Text *_txtTitle;
	bool _first;
	RuleBaseFacility *_lift;

	std::vector<RuleBaseFacility*> _accessLifts;
	entt::entity _window;
	Text *_txtHeader;
	TextList *_lstAccessLifts;
public:
	/// Creates the Place Lift state.
	PlaceLiftState(entt::handle newBaseId, Globe *globe, bool first);
	/// Handler for clicking the base view.
	void viewClick(Action *action);
	/// Handler for clicking the Access Lifts list.
	void lstAccessLiftsClick(Action *action);
};

}
