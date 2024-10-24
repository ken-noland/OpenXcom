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
#include "MultipleTargetsState.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Savegame/Target.h"
#include "../Savegame/Base.h"
#include "../Savegame/Craft.h"
#include "../Savegame/Ufo.h"
#include "GeoscapeState.h"
#include "ConfirmDestinationState.h"
#include "InterceptState.h"
#include "UfoDetectedState.h"
#include "GeoscapeCraftState.h"
#include "TargetInfoState.h"
#include "../Engine/Options.h"
#include "../Engine/Action.h"
#include "../Entity/Interface/Interface.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Multiple Targets window.
 * @param game Pointer to the core game.
 * @param targets List of targets to display.
 * @param craft Pointer to craft to retarget (NULL if none).
 * @param state Pointer to the Geoscape state.
 */
MultipleTargetsState::MultipleTargetsState(std::vector<Target*> targets, std::vector<Craft*> crafts, GeoscapeState *state, bool useCustomSound)
	: State("MultipleTargetsState", false), _targets(targets), _crafts(std::move(crafts)), _state(state), _useCustomSound(useCustomSound)
{
	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();

	if (_targets.size() > 1)
	{
		int winHeight = (int)(BUTTON_HEIGHT * _targets.size() + SPACING * (_targets.size() - 1) + MARGIN * 2);
		int winY = (200 - winHeight) / 2;
		int btnY = winY + MARGIN;

		// Create objects
		_window = factory.createWindow("windowName", this, 136, winHeight, 60, winY, WindowPopup::POPUP_VERTICAL);

		// Set palette
		setInterface("multipleTargets");

		add(_window, "window", "multipleTargets");

		// Set up objects
		setWindowBackground(_window, "multipleTargets");

		int y = btnY;
		for (size_t i = 0; i < _targets.size(); ++i)
		{
			TextButton *button = new TextButton(116, BUTTON_HEIGHT, 70, y);
			button->setText(_targets[i]->getName(getGame()->getLanguage()));
			button->onMouseClick((ActionHandler)&MultipleTargetsState::btnTargetClick);
			add(button, "button", "multipleTargets");

			_btnTargets.push_back(button);

			y += button->getHeight() + SPACING;
		}
		_btnTargets[0]->onKeyboardPress((ActionHandler)&MultipleTargetsState::btnCancelClick, Options::keyCancel);

		centerAllSurfaces();
	}
}

/**
 *
 */
MultipleTargetsState::~MultipleTargetsState()
{

}

/**
 * Resets the palette and ignores the window
 * if there's only one target.
 */
void MultipleTargetsState::init()
{
	if (_targets.size() == 1)
	{
		popupTarget(*_targets.begin());
	}
	else
	{
		State::init();
	}
}

/**
 * Displays the right popup for a specific target.
 * @param target Pointer to target.
 */
void MultipleTargetsState::popupTarget(Target *target)
{
	getGame()->popState();
	if (_crafts.size() == 0)
	{
		Base* b = dynamic_cast<Base*>(target);
		Craft* c = dynamic_cast<Craft*>(target);
		Ufo* u = dynamic_cast<Ufo*>(target);
		if (b != 0)
		{
			getGame()->pushState(new InterceptState(_state->getGlobe(), _useCustomSound, b));
		}
		else if (c != 0)
		{
			getGame()->pushState(new GeoscapeCraftState(c, _state->getGlobe(), 0, _useCustomSound));
		}
		else if (u != 0)
		{
			getGame()->pushState(new UfoDetectedState(u, _state, false, u->getHyperDetected()));
		}
		else
		{
			getGame()->pushState(new TargetInfoState(target, _state->getGlobe()));
		}
	}
	else
	{
		getGame()->pushState(new ConfirmDestinationState(_crafts, target));
	}
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void MultipleTargetsState::btnCancelClick(Action *)
{
	getGame()->popState();
}

/**
 * Pick a target to display.
 * @param action Pointer to an action.
 */
void MultipleTargetsState::btnTargetClick(Action *action)
{
	for (size_t i = 0; i < _btnTargets.size(); ++i)
	{
		if (action->getSender() == _btnTargets[i])
		{
			popupTarget(_targets[i]);
			break;
		}
	}
}

}
