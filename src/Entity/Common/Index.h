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

namespace OpenXcom {

/**
 * @brief compont that holds an entities position within a "virtual" index.
 */
struct Index {
	size_t index;
};

class IndexPositionSystem
{
	entt::registry& _registry;

public:
	IndexPositionSystem(entt::registry& registry) : _registry(registry) { }

	template<typename... Components>
	entt::handle findByIndex(size_t index)
	{
		std::ranges::range auto view = _registry.view<Index, Components>().each();
		auto matchingIndex = [index](const auto& each) { return std::get<1>(each).index == index; };
		auto findResult = std::ranges::find_if(view, matchingIndex);
		return findResult != view.end() ? entt::handle(_registry, std::get<0>(*findResult)) : entt::handle();
	}
};

}
