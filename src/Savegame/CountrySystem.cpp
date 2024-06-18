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
#include <numeric>
#include <ranges>
#include "CountrySystem.h"
#include "Country.h"
#include "../Mod/RuleCountry.h"
#include "../Engine/Registry.h"

namespace OpenXcom::CountrySystem
{

/**
* @brief finds a Country entity by name.
* @param registry the global entity registry
* @param countryName the name of the country to find
* @return the entity with a matching country name, or entt::null if not found
*/
static entt::entity findCountry(const entt::registry& registry, const std::string& countryName)
{
	auto countriesView = registry.view<Country>();
	auto countryNameFilter = [&](auto entity) { return countriesView.get<Country>(entity).getRules()->getType() == countryName; };
	auto find_itterator = std::ranges::find_if(countriesView, countryNameFilter);
	return find_itterator != countriesView.end() ? *find_itterator : entt::null;
}

/**
* @brief finds a Country entity by lat/long.
* @param registry the global component registry
* @param lon the longitude point to search for
* @param lat the lattitude point to search for
* @return id of the first country that has the point, or entt::null if not found
*/
entt::entity locateCountry(const entt::registry& registry, double lon, double lat)
{
	auto countryView = registry.view<Country>();
	auto containsPointFilter = [&](const auto& entity) { return countryView.get<Country>(entity).getRules()->contains(lon, lat); };
	auto findItterator = std::ranges::find_if(countryView, containsPointFilter);
	return findItterator != countryView.end() ? *findItterator : entt::null;
	
}

/**
 * @brief finds a Country that contains a Locatable object
 * @tparam Locatable the type to locate. Must implement getLatitude and getLongitude
 * @param registry the global component registry
 * @param locatable the object who's location to check.
 * @return id of the first country that has the locatable, or entt::null if not found
*/
template<typename Locatable>
entt::entity locateCountry(const entt::registry& registry, const Locatable& locatable)
{
	return locateCountry(registry, locatable.getLatitude(), locatable.getLongitude());
}

/**
 * Adds up the monthly funding of all the countries.
 * @param registry the global entity registry
 * @return Total funding.
 */
static int64_t getCountriesMonthlyFundingTotal()
{
	auto countryView = getRegistry().list<Country>();
	auto sumByLambda = [](int64_t total, const Country& country) { return total + country.getFunding().back(); };
	return std::accumulate(countryView.begin(), countryView.end(), 0, sumByLambda);
}

}
