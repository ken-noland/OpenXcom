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
#include "UfoFactory.h"
#include <entt/entt.hpp>
#include "../Common/GeoPosition.h"
#include "../Common/GeoVelocity.h"
#include "../Common/Name.h"
#include "../Common/Type.h"
#include "../../Mod/RuleUfo.h"
#include "../../Savegame/Ufo.h"

namespace OpenXcom
{

/**
 * @brief Creates a new ufo
 * @param ruleUfo the type of UFO
 * @param uniqueId id unique to this ufo, set to 0 to disable
 * @param hunterKillerPercentage percentage chance this ufo should be a hunter killer
 * @param huntMode determines target preference
 * @param huntBehavior determines fleet/fight behavior in dogfight
 * @return a handle wrapping the registry and the Ufo entity
 */
entt::handle UfoFactory::create(const RuleUfo& ruleUfo, int uniqueId, int hunterKillerPercentage, int huntMode, int huntBehavior)
{
	entt::entity ufoId = _registry.create();
	_registry.emplace<Ufo>(ufoId, ruleUfo, uniqueId, hunterKillerPercentage, huntMode, huntBehavior);

	_registry.emplace<NameComponent>(ufoId, "");

	_registry.emplace<Type>(ufoId, ruleUfo.getType());

	_registry.emplace<GeoPosition>(ufoId, 0.0, 0.0);

	_registry.emplace<GeoVelocity>(ufoId, 0.0, 0.0);

	return entt::handle(_registry, ufoId);
}

}
