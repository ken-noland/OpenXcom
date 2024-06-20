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
#include <entt/entt.hpp>
#include "../Common/IndexPosition.h"
#include "../Common/GeoPosition.h"
#include "../Common/GeoVelocity.h"
#include "../../Mod/Mod.h"

namespace OpenXcom
{

/**
 * @brief Creates a new base
 * @param global mod reference
 * @return a new base
 */
entt::handle BaseFactory::create(Mod& mod)
{
	entt::entity baseId = _registry.create();
	Base& base = _registry.emplace<Base>(baseId, mod);

	_registry.emplace<GeoPosition>(baseId, 0.0, 0.0);

	// velocity!? Well, why not. Costs little if it does not move, and if we want to let them move later, we can!
	_registry.emplace<GeoVelocity>(baseId, 0.0, 0.0);

	// size points to one past the last index naturally.
	size_t index = _registry.storage<Base>().size();
	IndexPosition& position = _registry.emplace<IndexPosition>(baseId, index);

	return entt::handle(registry, baseId);
};

}
