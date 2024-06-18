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
#include <concepts>
#include <numeric>
#include <ranges>
#include <entt/entt.hpp>

namespace OpenXcom
{

class Registry
{
private:
	entt::registry _registry{};
public:
	// Gets access to the underlying registry.
	[[nodiscard]] entt::registry& raw() { return _registry; }
	// Gets access to the underlying registry.
	[[nodiscard]] const entt::registry& raw() const { return _registry; }

	/**
	 * @brief Lists all Components in the registry in a const context
	 * @tparam Component the type of componet to list.
	 * @return a range of const Component references in the registry
	 */
	template<typename Component>
	[[nodiscard]] std::ranges::range auto list() const
	{
		return _registry.view<const Component>().each() | std::views::elements<1>;
	}

	/**
	 * @brief Lists all Components in the registry
	 * @tparam Component the type of componet to list.
	 * @return a range of Component references in the registry
	 */
	template<typename Component>
	[[nodiscard]] std::ranges::range auto list()
	{
		return _registry.view<Component>().each() | std::views::elements<1>;
	}

	/**
	 * @brief Lists with a filter all Components in the registry in a const context
	 * @tparam Component the type of componet to list and filter.
	 * @param predicate the const Component predicate to use to filter the list
	 * @return a range of const Component references in the registry that pass the predicate
	 */
	template<typename Component>
	[[nodiscard]] std::ranges::range auto list(std::predicate<const Component> auto predicate) const
	{
		return list<const Component>() | std::views::filter(predicate);
	}

	/**
	 * @brief Lists with a filter all Components in the registry
	 * @tparam Component the type of componet to list and filter.
	 * @param predicate the const Component predicate to use to filter the list
	 * @return a range of Component references in the registry that pass the predicate
	 */
	template<typename Component>
	[[nodiscard]] std::ranges::range auto list(std::predicate<const Component> auto predicate)
	{
		return list<Component>() | std::views::filter(predicate);
	}

	// Gets the size of the Component pool
	template<typename Component>
	[[nodiscard]] size_t size() const { return _registry.view<Component>().size(); }

	// Gets the first entity in the Components... pool
	template<typename... Components>
	[[nodiscard]] entt::entity front() const { return _registry.view<Components>().front(); }

	// Gets the first Component in the registry, if any, in a const context.
	template<typename Component>
	[[nodiscard]] const Component* frontValue() const;

	// Gets the first Component in the registry, if any, in a mutable context.
	template<typename Component>
	[[nodiscard]] Component* frontValue();

	// Gets the last Component in the registry, if any, in a const context.
	template<typename Component>
	[[nodiscard]] const Component* backValue() const;

	// Gets the last Component in the registry, if any, in a mutable context.
	template<typename Component>
	[[nodiscard]] Component* backValue();

	// Gets the index of id, based on its position in Components... registry, or -1 if not found.
	template<typename... Components>
	int index(entt::entity id) const;

	// Gets the entity in offset position from the front in the Components storage
	template<typename... Components>
	[[nodiscard]] entt::entity next(int offset) const;

	// Gets the component in offset position from the front in the Components storage in a const context
	template<typename Component>
	[[nodiscard]] const Component* nextValue(int offset) const;

	// Gets the component in offset position from the front in the Components storage in a mutable context
	template<typename Component>
	[[nodiscard]] Component* nextValue(int offset);

	// Creates a new entity and emplaces a new Component to it
	template<typename Component, typename... ConstructorArgs>
	Component& createAndEmplace(ConstructorArgs&&... args);

	// Destroys the entity with a matching component instance.
	template<typename Component>
	bool destroy(Component& component);

	// Gets the id of the entity that component points to.
	template<typename Component>
	[[nodiscard]] entt::entity find(Component* component) const;

	// Finds the first entity that meets predicate
	template<typename... Components>
	[[nodiscard]] entt::entity find_if(std::predicate<entt::entity> auto predicate) const;

	// Finds the first component that meets predicate within a const context
	template<typename Component>
	[[nodiscard]] const Component* findValue_if(std::predicate<const Component&> auto predicate) const;

	// Finds the first component that meets predicate within a mutable context
	template<typename Component>
	[[nodiscard]] Component* findValue_if(std::predicate<Component> auto predicate);

	// Finds the first component that has a matching rule name in a const context
	template<typename Component>
	[[nodiscard]] Component* findValueByName(const std::string& ruleType) const;

	// Finds the first component that has a matching rule name in a mutable context
	template<typename Component>
	[[nodiscard]] Component* findValueByName(const std::string& ruleType);

	// Gets the index of entity by its position in the registry store.
	template<typename... Components>
	[[nodiscard]] int getEntityIndex(entt::entity entity);

	// Adds up the totals of functions invoked against all Components in the registry
	template<typename Component, typename ReturnType, typename... Funcs>
	[[nodiscard]] ReturnType totalBy(Funcs... funcs);
};

Registry& getRegistry();

}
