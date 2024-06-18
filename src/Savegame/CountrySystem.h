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
#include <string>
#include <entt/entt.hpp>

namespace OpenXcom
{

namespace CountrySystem
{
	/// finds a country by lat/long
	[[nodiscard]] entt::entity locateCountry(const entt::registry& registry, double lon, double lat);
	/// finds a country containing a locatable
	template<typename Locatable>
	[[nodiscard]] entt::entity locateCountry(const entt::registry& registry, const Locatable& locatable);
	/// gets the total funding of all countries
	[[nodiscard]] int64_t getCountriesMonthlyFundingTotal();
}
}
