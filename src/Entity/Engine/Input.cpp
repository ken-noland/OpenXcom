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
#include "Input.h"
#include "Hierarchical.h"
#include "Surface.h"
#include "../../Engine/Action.h"

namespace OpenXcom
{

bool InputHandlerSystem::handle(const entt::handle& entity, Action* action) const
{
	//if (!_visible || _hidden)
	//	return;

	//action->setSender(this);

	//if (action->getDetails()->type == SDL_MOUSEBUTTONUP || action->getDetails()->type == SDL_MOUSEBUTTONDOWN)
	//{
	//	action->setMouseAction(action->getDetails()->button.x, action->getDetails()->button.y, getX(), getY());
	//}
	//else if (action->getDetails()->type == SDL_MOUSEMOTION)
	//{
	//	action->setMouseAction(action->getDetails()->motion.x, action->getDetails()->motion.y, getX(), getY());
	//}

	//if (action->isMouseAction())
	//{
	//	if ((action->getAbsoluteXMouse() >= getX() && action->getAbsoluteXMouse() < getX() + getWidth()) &&
	//		(action->getAbsoluteYMouse() >= getY() && action->getAbsoluteYMouse() < getY() + getHeight()))
	//	{
	//		if (!_isHovered)
	//		{
	//			_isHovered = true;
	//			mouseIn(action, state);
	//		}
	//		if (_listButton && action->getDetails()->type == SDL_MOUSEMOTION)
	//		{
	//			_buttonsPressed = SDL_GetMouseState(0, 0);
	//			for (Uint8 i = 1; i <= NUM_BUTTONS; ++i)
	//			{
	//				if (isButtonPressed(i))
	//				{
	//					action->getDetails()->button.button = i;
	//					mousePress(action, state);
	//				}
	//			}
	//		}
	//		mouseOver(action, state);
	//	}
	//	else
	//	{
	//		if (_isHovered)
	//		{
	//			_isHovered = false;
	//			mouseOut(action, state);
	//			if (_listButton && action->getDetails()->type == SDL_MOUSEMOTION)
	//			{
	//				for (Uint8 i = 1; i <= NUM_BUTTONS; ++i)
	//				{
	//					if (isButtonPressed(i))
	//					{
	//						setButtonPressed(i, false);
	//					}
	//					action->getDetails()->button.button = i;
	//					mouseRelease(action, state);
	//				}
	//			}
	//		}
	//	}


	InputHandlerComponent* inputHandlerComponent = entity.try_get<InputHandlerComponent>();
	if (inputHandlerComponent)
	{
		// check for a screen rect component
		ScreenRectComponent* screenRectComponent = entity.try_get<ScreenRectComponent>();
		if (screenRectComponent)
		{
			// check if the action is within the screen rect
			//if (!screenRectComponent->contains())
			//{
			//	return;
			//}
		}

		inputHandlerComponent->_onAction.call(action);
	}

	// if the entity has a hierarchy component, propagate the action to all children
	HierarchyComponent* hierarchyComponent = entity.try_get<HierarchyComponent>();
	if (hierarchyComponent)
	{
		for (const entt::handle& child : hierarchyComponent->_children)
		{
			handle(child, action);
		}
	}

	return true;
}

}
