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
#include <ranges>
#include <entt/entt.hpp>
#include "Country.h"
#include "Region.h"
#include "../Engine/Registry.h"
#include "../Mod/RuleCountry.h"
#include "../Mod/RuleRegion.h"

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
	/**
 	 * @brief finds the first area entity that contains lat/long
 	 * @tparam Area the type of area to find, either Region or Country
 	 ** @param lon the longitude point to search for
 	 * @param lat the lattitude point to search for
 	 * @return the entity with a matching country name, or entt::null if not found
 	 */
	template<typename Area>
	entt::entity locate(double lon, double lat)
	{
		auto areaView = getRegistry().raw().view<Area>().each();
		auto containsPoint = [lon, lat](const std::tuple<entt::entity, Area>& each) { return std::get<1>(each).getRules()->contains(lon, lat); };
		auto findResult = std::ranges::find_if(areaView, containsPoint);
		return findResult != areaView.end() ? std::get<0>(*findResult) : entt::null;
	}

	/**
	 * @brief finds the first area that contains a locatable
	 * @tparam Area the type of area to find, either Region or Country
	 * @tparam Locatable the type to locate. Must implement getLatitude and getLongitude
	 * @param locatable the object who's location to check.
	 * @return id of the first region that has the locatable, or entt::null if not found
	*/
	template<typename Area, typename Locatable>
	entt::entity locate(const Locatable& locatable) requires LocatableConcept<Locatable>
	{
		return locate<Area>(locatable.getLongitude(), locatable.getLatitude());
	}

	/**
	 * @brief finds the first area entity that contains lat/long
	 * @tparam Area the type of area to find, either Region or Country
	 * @param lon the longitude point to search for
	 * @param lat the lattitude point to search for
	 * @return pointer to the area that contains locatable, or nullptr if not found
	 */
	template<typename Area>
	Area* locateValue(double lon, double lat)
	{
		auto containsPoint = [lon, lat](const Area& area) { return area.getRules()->contains(lon, lat); };
		return getRegistry().findValue_if<Area>(containsPoint);
	}

	/**
	 * @brief finds the first area that contains locatable
	 * @tparam Area the type of area to find, either Region or Country
	 * @tparam Locatable the type to locate. Must implement getLatitude and getLongitude
	 * @param locatable the object who's location to check.
	 * @return id of the first region that has the locatable, or entt::null if not found
	*/
	template<typename Area, typename Locatable>
	Area* locateValue(const Locatable& locatable) requires LocatableConcept<Locatable>
	{
		return locateValue<Area>(locatable.getLongitude(), locatable.getLatitude());
	}

	/**
	 * @brief finds the first area that contains locatable
	 * @tparam Area the type of area to find, either Region or Country
	 * @tparam Locatable the type to locate. Must implement getLatitude and getLongitude
	 * @param locatable the object who's location to check.
	 * @return id of the first region that has the locatable, or entt::null if not found
	 */
	template<typename Area, typename Locatable>
	Area* locateValue(const Locatable* locatable) requires LocatableConcept<Locatable>
	{
		return locateValue<Area>(locatable->getLongitude(), locatable->getLatitude());
	}

	template <typename Area>
	using ActivityGetter = std::vector<int>& (Area::*)();

	template <typename Area, ActivityGetter<Area> Getter>
	bool addActivity(int amount, double lon, double lat)
	{
		entt::entity areaId = locate<Area>(lon, lat);
		if (areaId == entt::null) { return false; }
		(getRegistry().raw().get<Area>(areaId).*Getter)().back() += amount;
		return true;
	}

	/**
     * @brief Adds amount of Alien Activity to the current month for the first area containing lon/lat
     * @tparam Area the type of area to find, either Region or Country
     * @param registry the global entity registry
     * @param amount the amount activity to add.
     * @param lon the longitude coordinates
     * @param lat the lattitude coordinates
     * @return if the activity was added or not
    */
	template <typename Area>
	bool addAlienActivity(int amount, double lon, double lat)
	{
		return addActivity<Area, &Area::getActivityAlien>(amount, lon, lat);
	}

	/**
	 * @brief Adds amount of Alien Activity to the current month for the first area containing locatable
	 * @tparam Area the type of area to find, either Region or Country
	 * @tparam Locatable the type to locate. Must implement getLatitude and getLongitude
	 * @param registry the global entity registry
	 * @param amount the amount activity to add.
	 * @param locatable the object that provides the location
	 * @return if the activity was added or not
	*/
	template<typename Area, typename Locatable>
	bool addAlienActivity(int amount, const Locatable& locatable)
	{
		return addAlienActivity<Area>(amount, locatable.getLongitude(), locatable.getLatitude());
	}

	/**
	 * @brief Adds amount of Xcom Activity to the current month for the first region containing lon/lat
	 * @tparam Area the type of area to find, either Region or Country
	 * @param registry the global entity registry
	 * @param amount the amount activity to add.
	 * @param lon the longitude coordinates
	 * @param lat the lattitude coordinates
	 * @return if the activity was added or not
	*/
	template <typename Area>
	bool addXcomActivity(int amount, double lon, double lat)
	{
		return addActivity<Area, &Area::getActivityXcom>(amount, lon, lat);
	}

	/**
	 * @brief Adds amount of Xcom Activity to the current month for the first region containing locatable
	 * @tparam Area the type of area to find, either Region or Country
	 * @tparam Locatable the type to locate. Must implement getLatitude and getLongitude
	 * @param registry the global entity registry
	 * @param amount the amount activity to add.
	 * @param locatable the object that provides the location
	 * @return if the activity was added or not
	*/
	template<typename Area, typename Locatable>
	bool addXcomActivity(int amount, const Locatable& locatable)
	{
		return addXcomActivity<Area>(amount, locatable.getLongitude(), locatable.getLatitude());
	}

	/**
	 * @brief Adds amount of alien activty to the first country and the first region containing lat/lon
	 * @param amount the amount of activity to add
	 * @param lon the longitude point to search for
	 * @param lat the lattitude point to search for
	 * @return true if activity was added to country or region. False otherwise.
	*/
	inline bool addAlienActivityToCountryAndRegion(int amount, double lon, double lat)
	{
		return addAlienActivity<Region>(amount, lon, lat)
			|| addAlienActivity<Country>(amount, lon, lat);
	}

	/**
	 * @brief Adds amount of alien activty to the first country and the first region containing locatable
	 * @tparam Locatable the type to locate. Must implement getLatitude and getLongitude
	 * @param amount the amount of activity to add
	 * @param locatable the objects who's location should be used to search
	 * @return true if activity was added to country or region. False otherwise.
	*/
	template<typename Locatable>
	inline bool addAlienActivityToCountryAndRegion(int amount, const Locatable& locatable)
	{
		return addAlienActivity<Region>(amount, locatable)
			|| addAlienActivity<Country>(amount, locatable);
	}

	/**
	 * @brief Adds amount of xcom actiivty to the first country and the first region containing lat/lon
	 * @param amount the amount of activity to add
	 * @param lon the longitude point to search for
	 * @param lat the lattitude point to search for
	 * @return true if activity was added to country or region. False otherwise.
	*/
	inline bool addXcomActivityToCountryAndRegion(int amount, double lon, double lat)
	{
		return addXcomActivity<Region>(amount, lon, lat)
			|| addXcomActivity<Country>(amount, lon, lat);
	}

	/**
	 * @brief Adds amount of alien activty to the first country and the first region containing locatable
	 * @tparam Locatable the type to locate. Must implement getLatitude and getLongitude
	 * @param amount the amount of activity to add
	 * @param locatable the objects who's location should be used to search
	 * @return true if activity was added to country or region. False otherwise.
	*/
	template<typename Locatable>
	inline bool addXcomActivityToCountryAndRegion(int amount, const Locatable& locatable)
	{
		return addXcomActivity<Region>(amount, locatable)
			|| addXcomActivity<Country>(amount, locatable);
	}
}

}
