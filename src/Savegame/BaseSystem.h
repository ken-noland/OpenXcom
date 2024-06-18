#pragma once
/*
 * Copyright 2010-2024 OpenXcom Developers.
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
#include <entt/entt.hpp>

namespace OpenXcom
{

namespace BaseSystem
{
	// Gets the index of a given base entity.
	[[nodiscard]] int getBaseIndex(const entt::registry& registry, entt::entity baseId);
	// Adds up the cost of variable number of member functions for all bases.
	template<typename... Funcs>
	[[nodiscard]] static int64_t getBasesTotal(Funcs... funcs);
	// Gets the total maintiance cost for all bases.
	[[nodiscard]] int64_t getBasesMaintenanceCost();
	// Gets if an item is in storage at any base.
	[[nodiscard]] bool isItemInBaseStores(const entt::registry& registry, const std::string& itemType);
	// Gets if a facility is built at any base.
	[[nodiscard]] bool isFacilityBuilt(const entt::registry& registry, const std::string& facilityType);
	// Gets if a solider types is hired at any base.
	[[nodiscard]] bool isSoldierTypeHired(const entt::registry& registry, const std::string& soldierType);
}

}
