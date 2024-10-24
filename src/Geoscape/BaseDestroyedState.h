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
 * along with OpenXcom.  If not, see <http:///www.gnu.org/licenses/>.
 */
#include "../Engine/State.h"

namespace OpenXcom
{

class Base;

class Text;
class TextButton;
class TextList;
class Ufo;

/**
 * Screen that allows the player
 * to pick a target for a craft on the globe.
 */
class BaseDestroyedState : public State
{
private:
	entt::handle _window;
	Text *_txtMessage;
	TextButton *_btnOk;
	TextList *_lstDestroyedFacilities;
	Base *_base;
	bool _missiles, _partialDestruction;
public:
	/// Creates the Select Destination state.
	BaseDestroyedState(Base *base, const Ufo* ufo, bool missiles, bool partialDestruction);
	/// Cleans up the Select Destination state.
	~BaseDestroyedState();
	/// Handler for clicking the Cydonia mission button.
	void btnOkClick(Action *action);

};

}
