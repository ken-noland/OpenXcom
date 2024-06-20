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

namespace OpenXcom
{

InterfaceFactory::InterfaceFactory(entt::registry& registry, SurfaceFactory& sf)
	: _registry(registry), _surfaceFactory(sf)
{
}

InterfaceFactory::~InterfaceFactory()
{
}


entt::entity InterfaceFactory::createWindow(const std::string& name, State* state, int width, int height, int x, int y, WindowPopup popup)
{
	entt::entity entity = _surfaceFactory.createSurface(name, width, height, x, y);

	SurfaceComponent& sufaceComponent = _registry.get<SurfaceComponent>(entity);
	_registry.emplace<WindowComponent>(entity, sufaceComponent, state, popup);

	return entity;
}


}
