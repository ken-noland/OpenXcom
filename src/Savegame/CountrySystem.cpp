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
 * Adds up the monthly funding of all the countries.
 * @param registry the global entity registry
 * @return Total funding.
 */
int64_t getCountriesMonthlyFundingTotal()
{
	auto countryView = getRegistry().list<Country>();
	auto sumByLambda = [](int64_t total, const Country& country) -> int64_t	{ return total + (int64_t)country.getFunding().back(); };
	return std::accumulate(countryView.begin(), countryView.end(), (int64_t)0, sumByLambda);
}

}
