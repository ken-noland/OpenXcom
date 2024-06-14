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
#include <vector>
#include <entt/entt.hpp>

namespace OpenXcom {

class EntityManager {
public:
	EntityManager() : _registry() {}

	/**
	 * @brief Creates and register a new entity of Component type by calling its constructor with components.
	 * @return The entity ID that was created.
	*/
	template<typename... Components>
	entt::entity createEntity(Components&&... components) {
		entt::entity entity = _registry.create();
		_registry.emplace<Components...>(entity, std::forward<Components>(components)...);
		return entity;
	}

	// Destroy an existing entity
	void destroyEntity(entt::entity entity) { _registry.destroy(entity); }	

	// Get an component from an entity
	template<typename Component>
	Component& getComponent(entt::entity entity) { return _registry.get<Component>(entity); }

	/// return a view of all entities of a given type
	template<typename Component>
	auto getAll() { return _registry.view<Component>(); }

	template<typename Component>
	entt::basic_view<entt::entity, entt::exclude_t<>, Component> getAll() const { return _registry.view<Component>(); }

private:
	entt::registry _registry;
};

}
