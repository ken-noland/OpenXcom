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
#include "Interface.h"

#include "Window.h"
#include "ArrowButton.h"
#include "TextButton.h"

#include "../Engine/Tickable.h"
#include "../Engine/Drawable.h"

namespace OpenXcom
{

InterfaceFactory::InterfaceFactory(entt::registry& registry, SurfaceFactory& sf)
	: _registry(registry), _surfaceFactory(sf)
{
}

InterfaceFactory::~InterfaceFactory()
{
}

entt::entity InterfaceFactory::createArrowButton(const std::string& name, ArrowShape shape, int width, int height, int x, int y)
{
	entt::entity entity = _surfaceFactory.createSurface(name, width, height, x, y);

	SurfaceComponent& surfaceComponent = _registry.get<SurfaceComponent>(entity);
	DrawableComponent& drawableComponent = _registry.get<DrawableComponent>(entity);
	TickableComponent& tickableComponent = _registry.emplace<TickableComponent>(entity);
	_registry.emplace<ArrowButtonComponent>(entity, surfaceComponent, tickableComponent, drawableComponent, shape);

	return entity;
}

entt::entity InterfaceFactory::createTextButton(const std::string& name, const std::string& text, int width, int height, int x, int y, std::function<void(Action*)> onClickCallback)
{
	entt::entity entity = _surfaceFactory.createSurface(name, width, height, x, y);

	_registry.emplace<TextButtonComponent>(entity);

	return entity;
}

entt::entity InterfaceFactory::createWindow(const std::string& name, State* state, int width, int height, int x, int y, WindowPopup popup)
{
	entt::entity entity = _surfaceFactory.createSurface(name, width, height, x, y);

	SurfaceComponent& surfaceComponent = _registry.get<SurfaceComponent>(entity);
	DrawableComponent& drawableComponent = _registry.get<DrawableComponent>(entity);
	TickableComponent& tickableComponent = _registry.emplace<TickableComponent>(entity);
	_registry.emplace<WindowComponent>(entity, surfaceComponent, tickableComponent, drawableComponent, state, popup);

	return entity;
}



}
