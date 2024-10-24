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

namespace OpenXcom
{

class Base;
class BaseView;
class BaseFacility;
class TextButton;
class Text;

/**
 * Window shown when the player tries to
 * dismantle a facility.
 */
class DismantleFacilityState : public State
{
private:
	Base *_base;
	BaseView *_view;
	BaseFacility *_fac;

	TextButton *_btnOk, *_btnCancel;
	entt::handle _window;
	Text *_txtTitle, *_txtFacility, *_txtRefundValue;
public:
	/// Creates the Dismantle Facility state.
	DismantleFacilityState(Base *base, BaseView *view, BaseFacility *fac);
	/// Cleans up the Dismantle Facility state.
	~DismantleFacilityState();
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Cancel button.
	void btnCancelClick(Action *action);
};

}
