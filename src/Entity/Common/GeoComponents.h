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
#include "../../Mod/RuleCountry.h"
#include "../../Mod/RuleRegion.h"
#include "../../Savegame/Country.h"
#include "../../Savegame/Region.h"

namespace OpenXcom
{

/**
 * @brief object with a location on the geoscape. Like UFO or bases. In radians.
 */
struct GeoPosition
{
	double latitude = 0.0;
	double longitude = 0.0;

	// doesn't matter now but probably want some way to throttle these updates
	// double distanceMovedSinceLastUpdate; 
	Region* region = nullptr;
	Country* country = nullptr;

	GeoPosition operator+(const GeoPosition& other) const
	{
		return GeoPosition{
			.latitude  = this->latitude  + other.latitude,
			.longitude = this->longitude + other.longitude,
		};
	}

	static constexpr std::string_view NODE_NAME = "name";
};

/**
 * @brief entity with a velocity on the geoscape
 */
struct GeoVelocity
{
	double bearing; // Bearing in degrees from north
	double speed;   // Speed in nm/???
};

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
	Area* locate(const GeoPosition& position)
	{
		auto areaView = _registry.view<Area>().each();

		auto containsPoint = [&position](const std::tuple<entt::entity, Area>& each) {
			return std::get<1>(each).getRules()->contains(position.longitude, position.latitude);
			};

		auto findResult = std::ranges::find_if(areaView, containsPoint);
		return findResult != areaView.end() ? &std::get<1>(*findResult) : nullptr;
	}

	void updateAllRegionsAndCountries()
	{
		for (auto&& [entity, geoPosition] : _registry.view<GeoPosition>().each())
		{
			Country* currentCountry = geoPosition.country;
			Region* currentRegion = geoPosition.region;

			Country* newCountry = locate<Country>(geoPosition);
			Region* newRegion = locate<Region>(geoPosition);

			if ((currentCountry != newCountry) || (currentRegion != newRegion))
			{
				geoPosition.country = newCountry;
				geoPosition.region = newRegion;
				_registry.replace<GeoPosition>(entity, geoPosition);
			}
		}
	}
};

}

namespace YAML
{

	template<>
	struct convert<OpenXcom::GeoPosition> {
		static Node encode(const OpenXcom::GeoPosition& geoPosition) {
			Node node;
			node[OpenXcom::GeoPosition::NODE_NAME.data()]["latitude"] = geoPosition.latitude;
			node[OpenXcom::GeoPosition::NODE_NAME.data()]["longitude"] = geoPosition.longitude;
			return node;
		}

		static bool decode(const Node& node, OpenXcom::GeoPosition& geoPosition) {
			if (!node.IsMap() || node.size() != 2) {
				return false;
			}

			geoPosition.latitude  = node["latitude"].as<double>();
			geoPosition.longitude = node["longitude"].as<double>();
			return true;
		}
	};
}
