#pragma once
/*
 * Copyright 2024-2024 OpenXcom Developers.
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
#include "../Common/GeoPosition.h"
#include "../../Mod/RuleCountry.h"
#include "../../Mod/RuleRegion.h"
#include "../../Savegame/Country.h"
#include "../../Savegame/Region.h"

namespace OpenXcom {

class GeoSystem
{
	entt::registry& _registry;

public:
	GeoSystem(entt::registry& registry) : _registry(registry) { }

	/**
	 * @brief finds the first area entity that contains location, if any
	 * @tparam Area the type of area to find, either Region or Country
	 * @param position the position to search for
	 * @return the first area with a matching point, or nullptr if not found
	 */
	template<typename Area>
	entt::handle locate(const GeoPosition& position)
	{
		auto areaView = _registry.view<Area>().each();

		auto containsPoint = [&position](const std::tuple<entt::entity, Area>& each) {
			return std::get<1>(each).getRules()->contains(position.longitude, position.latitude);
			};

		auto findResult = std::ranges::find_if(areaView, containsPoint);
		return findResult != areaView.end() ? entt::handle(_registry, std::get<0>(*findResult)) : entt::handle();
	}

	/**
	 * @brief Updates the current region and country value for all geopositions.
	 */
	void updateAllRegionsAndCountries()
	{
		for (auto&& [entity, geoPosition] : _registry.view<GeoPosition>().each())
		{
			entt::handle currentCountry = geoPosition.country;
			entt::handle currentRegion  = geoPosition.region;

			entt::handle newCountry = locate<Country>(geoPosition);
			entt::handle newRegion  = locate<Region>(geoPosition);

			if ((currentCountry.entity() != newCountry.entity()) || (currentRegion.entity() != newRegion.entity()))
			{
				geoPosition.country = newCountry;
				geoPosition.region  = newRegion;
				_registry.replace<GeoPosition>(entity, geoPosition);
			}
		}
	}
};

}
