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
#include <concepts>
#include <entt/entt.hpp>
#include <ranges>
#include "..\Savegame\Country.h"
#include "..\Mod\RuleCountry.h"

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
	 * @return an itterable of const Component references in the registry
	 */
	template<typename Component>
	[[nodiscard]] auto list() const
	{
		return _registry.view<const Component>() | std::views::elements<1>();
	}

	/**
	 * @brief Lists all Components in the registry in a mutable context
	 * @tparam Component the type of componet to list.
	 * @return an itterable of mutable Component references in the registry
	 */
	template<typename Component>
	[[nodiscard]] auto list()
	{
		return _registry.view<Component>().each() | std::views::elements<1>();
	}

	/**
	 * @brief Lists with a filter all Components in the registry in a const context
	 * @tparam Component the type of componet to list and filter.
	 * @param predicate the const Component predicate to use to filter the list
	 * @return an itterable of const Component references in the registry that pass the predicate
	 */
	template<typename Component>
	[[nodiscard]] auto list(std::predicate<const Component> auto predicate) const
	{
		return list<Component>() | std::views::filter(predicate);
	}

	/**
	 * @brief Lists with a filter all Components in the registry in a mutable context
	 * @tparam Component the type of componet to list and filter.
	 * @param predicate the const Component predicate to use to filter the list
	 * @return an itterable of mutable Component references in the registry that pass the predicate
	 */
	template<typename Component>
	[[nodiscard]] auto list(std::predicate<const Component> auto predicate)
	{
		return list<Component>() | std::views::filter(predicate);
	}

	/**
	 * @brief Creates a new entity and emplaces a new Component to it
	 * @tparam Component the type of Component to emplace
	 * @tparam ...ConstructorArgs the type of arguments to pass to the Component constructor
	 * @param ...args the arguments to be forward to the Component constructor
	 * @return A reference to the newly created Component
	*/
	template<typename Component, typename... ConstructorArgs>
	Component& createAndEmplace(ConstructorArgs&&... args)
	{
		return _registry.emplace<Component>(_registry.create(), std::forward<ConstructorArgs>(args)...);
	}

	/**
	 * @brief Finds the first entity that meets predicate
	 * @tparam ...Components the set of components the entity must contain
	 * @param predicate The predicate used to find the entity
	 * @return the first entity that meets the critieria, or entt::null if no matching enttity found.
	 */
	template<typename... Components>
	[[nodiscard]] entt::entity find_if(std::predicate<entt::entity> auto predicate) const
	{
		auto view = _registry.view<Components...>();
		auto findIterator = std::ranges::find_if(view.begin(), view.end(), predicate);
		return findIterator != view.end() ? &(*findIterator) : entt::null;
	}

	/**
	 * @brief Finds the first component that meets predicate within a const context
	 * @tparam Component the Components the entity must contain, and is returned.
	 * @param predicate The predicate used to find the entity
	 * @return a const pointer to the first component that meets the critieria, or nullptr if no matching component is found
	 */
	template<typename Component>
	[[nodiscard]] const Component* findValue_if(std::predicate<const Component> auto predicate) const
	{
		auto view = list<const Component>();
		auto findItterator = std::ranges::find_if(view, predicate);
		return findItterator != view.End() ? &(*findIterator) : nullptr;
	}

	/**
	 * @brief Finds the first component that meets predicate within a mutable context
	 * @tparam Component the Components the entity must contain, and is returned.
	 * @param predicate The predicate used to find the entity
	 * @return a pointer to the first component that meets the critieria, or nullptr if no matching component is found
	*/
	template<typename Component>
	[[nodiscard]] Component* findValue_if(std::predicate<Component> auto predicate)
	{
		auto view = list<Component>();
		auto findItterator = std::ranges::find_if(view, predicate);
		return findItterator != view.End() ? &(*findIterator) : nullptr;
	}

	/**
	 * @brief Finds the first component that has a matching rule name in a const context
	 * @tparam Component the Components the entity must contain, and is returned.
			   Must have a method getRule()->getType() which returns a string reference.
	 * @param ruleType the name of the rule to match
	 * @return A pointer to a const instance of the first Component with a matching name, or nullptr if no match is found
	*/
	template<typename Component>
	[[nodiscard]] Component* findValueByName(const std::string& ruleType) const
	{
		auto view = list<Component>();
		auto typePredicate = [&ruleType](Component& component) { return ruleType == component.getRules()->getType(); };
		auto findIterator = std::ranges::find_if(view, typePredicate);
		return findIterator != view.end() ? &(*findIterator) : nullptr;
	}

	/**
	 * @brief Finds the first component that has a matching rule name in a mutable context
	 * @tparam Component the Components the entity must contain, and is returned.
			   Must have a method getRule()->getType() which returns a string reference.
	 * @param ruleType the name of the rule to match
	 * @return A pointer to the first Component with a matching name, or nullptr if no match is found
	*/
	template<typename Component>
	[[nodiscard]] Component* findValueByName(const std::string& ruleType)
	{
		auto view = list<Component>();
		auto typePredicate = [&ruleType](Component& component) { return ruleType == component.getRules()->getType(); };
		auto findIterator = std::find_if(view.begin(), view.end(), typePredicate);
		return findIterator != view.end() ? &(*findIterator) : nullptr;
	}

	/**
	 * @brief Gets the index of entity by its position in the registry store.
	 * @tparam ...Components the set of components the entity must contain
	 * @param entity the entity whos index to find
	 * @return the index of entity, or -1 if it is not found.
	 */
	template<typename... Components>
	static int getEntityIndex(entt::entity entity)
	{
		auto view = list<Components...>();
		auto findIterator = std::find(list.begin(), list.end(), entity);
		return findIterator != list.end() ? std::distance(list.begin(), findIterator) : -1;
	}

	/**
	 * @brief Adds up the totals of functions invoked against all Components in the registry
	 * @tparam Component the type of componet to sum
	 * @tparam ReturnType the type of values to sum, must be addable
	 * @tparam ...Funcs The functions to sum. Should take a Component and return a ReturnValue
	 * @param registry The global component registry
	 * @param ...funcs the member functions to sum
	 * @return the sum total of all member functions for all bases.
	*/
	template<typename Component, typename ReturnType, typename... Funcs>
	static ReturnType totalBy(Funcs... funcs)
	{
		auto componentList = list<Component>();
		auto sumMemberFunctions = [&funcs...](ReturnType total, const Component& component) {
			return total + (... + std::invoke(funcs, component));
		};
		return std::accumulate(componentList.begin(), componentList.end(), ReturnType{}, sumMemberFunctions);
	}
};

Registry& getRegistry();

}
