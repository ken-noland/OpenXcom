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
#include "CountryFactory.h"
#include <entt/entt.hpp>
#include "../Common/Type.h"
#include "../../Savegame/Country.h"
#include "../../Mod/RuleCountry.h"

namespace OpenXcom
{

/**
 * @brief Creates a new Country
 * @param ruleCountry type of this country
 * @param generateNewFunding indicates if default funding should be generated for this country.
 * @return a new Country entity
 */
entt::handle CountryFactory::create(RuleCountry& ruleCountry, bool generateNewFunding)
{
	entt::entity countryId = _registry.create();
	_registry.emplace<Country>(countryId, &ruleCountry, generateNewFunding);

	_registry.emplace<Type>(countryId, ruleCountry.getType());

	return entt::handle(_registry, countryId);
};

}
