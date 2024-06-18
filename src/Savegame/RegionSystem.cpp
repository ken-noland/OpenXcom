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
#include <ranges>
#include "Base.h"
#include "RegionSystem.h"
#include "Region.h"
#include "../Mod/RuleRegion.h"

namespace OpenXcom::RegionSystem
{

/**
* @brief finds a Region entity by name.
* @param registry the global entity registry
* @param regionName the name of the country to find
* @return the entity with a matching region name, or entt::null if not found
*/
static entt::entity findRegion(const entt::registry& registry, const std::string& regionName)
{
	auto regionsView = registry.view<Region>();
	auto countryNameFilter = [&](auto entity) { return regionsView.get<Region>(entity).getRules()->getType() == regionName; };
	auto find_itterator = std::ranges::find_if(regionsView, countryNameFilter);
	return find_itterator != regionsView.end() ? *find_itterator : entt::null;
}

template<typename AreaType>
static entt::entity find(const entt::registry& registry, const std::string& regionName)
{
	auto regionsView = registry.view<AreaType>();
	auto countryNameFilter = [&](auto entity) { return regionsView.get<Region>(entity).getRules()->getType() == regionName; };
	auto find_itterator = std::ranges::find_if(regionsView, countryNameFilter);
	return find_itterator != regionsView.end() ? *find_itterator : entt::null;
}

/**
* @brief finds a Region entity by lat/long.
* @param registry the global component registry
* @param lon the longitude point to search for
* @param lat the lattitude point to search for
* @return the entity with a matching country name, or entt::null if not found
*/
static entt::entity locateRegion(const entt::registry& registry, double lon, double lat)
{
	auto regionsView = registry.view<Region>();
	auto containsPointFilter = [&](const auto& entity) { return regionsView.get<Region>(entity).getRules()->insideRegion(lon, lat); };
	auto findItterator = std::ranges::find_if(regionsView, containsPointFilter);
	return findItterator != regionsView.end() ? *findItterator : entt::null;
}

/**
 * @brief finds a Region that contains a Locatable object
 * @tparam Locatable the type to locate. Must implement getLatitude and getLongitude
 * @param registry the global component registry
 * @param locatable the object who's location to check.
 * @return id of the first region that has the locatable, or entt::null if not found
*/
template<typename Locatable>
static entt::entity locateRegion(const entt::registry& registry, const Locatable& locatable)
{
	return locateRegion(registry, locatable.getLongitude(), locatable.getLatitude());
}

}
