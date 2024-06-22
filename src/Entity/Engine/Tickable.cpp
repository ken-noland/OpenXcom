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
#include "../../Engine/Game.h"
#include "../../Engine/Registry.h"
#include "../../Engine/Screen.h"
#include "../Interface/Window.h"
#include "Tickable.h"
#include "ECS.h"

namespace OpenXcom
{

TickableComponent::TickableComponent()
{
}

TickableComponent::~TickableComponent()
{
}

void TickableComponent::addTickable(const TickableCallback& tickable)
{
	_tickables.push_back(tickable);
}

void TickableComponent::tick()
{
	_tickables.call();
}

TickableSystem::TickableSystem()
{
}

TickableSystem::~TickableSystem()
{
}

void TickableSystem::tick(entt::entity& entity)
{
	if (getRegistry().raw().any_of<WindowComponent>(entity))
	{
		TickableComponent& tickableComponent = getRegistry().raw().get<TickableComponent>(entity);
		tickableComponent.tick();
	}
	else
	{
		Surface* surface = getRegistry().raw().get<SurfaceComponent>(entity).getSurface();
		surface->think();
	}
}

} // namespace OpenXcom
