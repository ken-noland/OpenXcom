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
#include "Drawable.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/Button.h"
#include "../../Engine/Game.h"
#include "../../Engine/Screen.h"
#include "../../Engine/Registry.h"
#include "ECS.h"

namespace OpenXcom
{

DrawableComponent::DrawableComponent()
{
}

DrawableComponent::~DrawableComponent()
{
}

void DrawableComponent::addDrawable(const DrawableCallback& drawable)
{
	_drawables.push_back(drawable);
}

void DrawableComponent::draw()
{
	_drawables.call();
}

DrawableSystem::DrawableSystem()
{
}

DrawableSystem::~DrawableSystem()
{
}

void DrawableSystem::draw(entt::handle& entity)
{
	//KN NOTE: Right, so, not everything has been migrated over to ECS yet, so this function is a bit mixed.

	// KN NOTE: A bit of a hack for now until we have more of the surface stuff
	//  moved over to ECS. Some are using the old draw method(blitting) whereas
	//  others are using the new DrawableComponent

	if (entity.any_of<WindowComponent>())
	{
		WindowSystem& windowSystem = getSystem<WindowSystem>();
		windowSystem.draw(entity);
	}
	else if (entity.any_of<ButtonComponent>())
	{
		ButtonSystem& buttonSystem = getSystem<ButtonSystem>();
		buttonSystem.draw(entity);
	}
	else if (entity.any_of<TextComponent>())
	{
		TextSystem& textSystem = getSystem<TextSystem>();
		textSystem.draw(entity);

		//DrawableComponent& drawableComponent = entity.get<DrawableComponent>();
		//drawableComponent.draw();

		//TODO: I'm not sure this is needed once we have the systems up and running
		//{
		//	SurfaceComponent& surfaceComponent = entity.get<SurfaceComponent>();
		//	ScreenRectComponent& screenRectComponent = entity.get<ScreenRectComponent>();
		//	Surface* surface = surfaceComponent.getSurface();
		//}
	}
	else
	{
		Surface* surface = entity.get<SurfaceComponent>().getSurface();
		surface->blit(getGame()->getScreen()->getSurface());
	}
}

void DrawableSystem::update()
{
}

} // namespace OpenXcom
