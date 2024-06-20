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
#include "RegionFactory.h"
#include <entt/entt.hpp>

namespace OpenXcom {

class Region;

/**
 * @brief Creates a new region entity
 * @param ruleRegion the type of this country
 * @return a handle to the new entity
 */
entt::handle RegionFactory::create(const RuleRegion& ruleRegion)
{
	entt::entity regionId = _registry.create();
	_registry.emplace<Region>(regionId);

	return entt::handle(_registry, regionId);
}

}
