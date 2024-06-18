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

namespace OpenXcom
{

/**
 * @brief defines a type that has a longitude and latitude
 */
template<typename T>
concept LocatableConcept = requires(T locatable) {
	{ locatable.getLongitude() } -> std::convertible_to<double>;
	{ locatable.getLatitude() } -> std::convertible_to<double>;
};

namespace AreaSystem
{
	/// locates the first area entity that contains lat/long
	template<typename AreaType>
	[[nodiscard]] entt::entity locate(double lon, double lat);
	/// locates the first area entity that contains locatable
	template<typename AreaType, typename Locatable>
	[[nodiscard]] entt::entity locate(const Locatable& locatable);
	/// locates the first area entity that contains lat/long
	template<typename AreaType>
	[[nodiscard]] AreaType* locateValue(double lon, double lat);
	/// locates the first area entity that contains locatable
	template<typename AreaType, typename Locatable>
	[[nodiscard]] AreaType* locateValue(const Locatable& locatable);

	bool addAlienActivityToCountryAndRegion(int amount, double lon, double lat);

	template<typename Locatable>
	bool addAlienActivityToCountryAndRegion(int amount, const Locatable& locatable);

	bool addXcomActivityToCountryAndRegion(int amount, double lon, double lat);
	template<typename Locatable>
	bool addXcomActivityToCountryAndRegion(int amount, const Locatable& locatable);

	template<typename AreaType>
	bool addAlienActivity(int amount, double lon, double lat);
	template<typename AreaType, typename Locatable>
	bool addAlienActivity(int amount, const Locatable& locatable);

	template<typename AreaType>
	bool addXcomActivity(int amount, double lon, double lat);
	template<typename AreaType, typename Locatable>
	bool addXcomActivity(int amoutn, const Locatable& locatable);
}

}
