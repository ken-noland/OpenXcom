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
#include "ECS.h"

//temp includes until I figure out where I want to populate the factory and system registries
#include "Drawable.h"
#include "Tickable.h"
#include "Surface.h"
#include "Hierarchical.h"

#include "../../Engine/Timer.h"

#include "../Interface/Interface.h"
#include "../Interface/Window.h"

namespace OpenXcom
{

ECS::ECS()
{
	// ---
	// Systems
	// 
	// Systems are updated in the order that they are registered.
	{
		// KN NOTE: I don't think we need these anymore
		registerSystem<TickableSystem>();
		registerSystem<DrawableSystem>();
	}

	registerSystem<HierarchySystem>();

	registerSystem<WindowSystem>();

	// Timers get processed last
	const TimeSystem& timeSystem = registerSystem<TimeSystem>();

	registerSystem<ProgressTimerSystem>(getRegistry().raw(), timeSystem);
	registerSystem<IntervalTimerSystem>();

	// ---
	// Factories

	registerFactory<SurfaceFactory>(_registry.raw());
	registerFactory<InterfaceFactory>(*this, nullptr); // KN TODO: When I get around to rewriting Mod, I'll need to pass it in here
}

ECS::~ECS()
{
}

void ECS::update()
{
	// Update all systems
	for (std::pair<const std::type_index, TypeErasedUpdatePtr>& pair : _systemRegistry)
	{
		TypeErasedUpdatePtr& system = pair.second;
		if (system.hasUpdate())
		{
			system.update();
		}
	}
}

} // namespace OpenXcom
