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

namespace RegionSystem
{
	[[nodiscard]]

	/// finds a region by name
	[[nodiscard]] entt::entity findRegion(const entt::registry& registry, const std::string& regionName);
	/// locates a region id by lat/long
	[[nodiscard]] entt::entity locateRegion(const entt::registry& registry, double lon, double lat);
	/// locates a region given a locateable object
	template<typename Locatable>
	[[nodiscard]] entt::entity locateRegion(const entt::registry& registry, const Locatable& locatable);
}

}
