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
#include "BaseFactory.h"
#include "../Common/Index.h"
#include "../Common/GeoPosition.h"
#include "../Common/GeoVelocity.h"
#include "../Common/Name.h"
#include "../../Savegame/Base.h"
#include "../../Mod/Mod.h"

namespace OpenXcom
{

/**
 * @brief Creates a new base
 * @param global mod reference
 * @return a new base
 */
entt::handle BaseFactory::create(const Mod& mod)
{
	entt::entity baseId = _registry.create();
	_registry.emplace<Base>(baseId, &mod);
	_registry.emplace<Index>(baseId, _nextBaseIndex++);
	_registry.emplace<NameComponent>(baseId, "");
	_registry.emplace<GeoPosition>(baseId, 0.0, 0.0);

	// velocity!? Well, why not. Costs little if it does not move, and if we want to let them move later, we can!
	_registry.emplace<GeoVelocity>(baseId, 0.0, 0.0);

	sortBases();
	
	return entt::handle(_registry, baseId);
}

entt::handle BaseFactory::create(const Mod& mod, const YAML::Node& node, SavedGame* save, bool newGame, bool newBattleGame)
{
	entt::entity baseId = _registry.create();

	Base& base = _registry.emplace<Base>(baseId, &mod);
	base.load(node, save, newGame, newBattleGame);

	_registry.emplace<Index>(baseId, _nextBaseIndex++);

	GeoPosition position{
		.latitude  = node["lat"].as<double>(0.0),
		.longitude = node["lon"].as<double>(0.0),
	};

	_registry.emplace<GeoPosition>(baseId, position);
	_registry.emplace<NameComponent>(baseId, node["name"].as<std::string>(""));

	// velocity!? Well, why not. Costs little if it does not move, and if we want to let them move later, we can!
	_registry.emplace<GeoVelocity>(baseId, 0.0, 0.0);

	sortBases();

	return entt::handle(_registry, baseId);
}

void BaseFactory::sortBases()
{
	auto baseSorter = [&](const entt::entity& lhs, const entt::entity& rhs) {
		return _registry.get<Index>(lhs).index < _registry.get<Index>(lhs).index; };
	_registry.sort<Base>(baseSorter);
}

}
