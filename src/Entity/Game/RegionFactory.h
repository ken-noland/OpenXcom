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

class RuleRegion;

/**
 * @brief Factory for creating game entities
 */
class RegionFactory
{
protected:
	entt::registry& _registry;

public:
	// creates a new base factory
	RegionFactory(entt::registry& registry) : _registry(registry) { }

	// creates a new region
	entt::handle create(const RuleRegion& ruleRegion);
};

}
