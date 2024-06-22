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

class RuleCountry;

/**
 * @brief Factory for creating countries
 */
class CountryFactory
{
protected:
	entt::registry& _registry;

public:
	// creates a new country factory
	CountryFactory(entt::registry& registry) : _registry(registry) { }

	// creates a new Country entity in the registry.
	entt::handle create(RuleCountry& ruleCountry, bool generateNewFunding = true);
};

}
