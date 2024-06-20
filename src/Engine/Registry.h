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

	/**
     * @brief Gets the first Component in the registry, if any, in a const context.
     * @tparam Component the type of component to retrieve
     * @return a const pointer to the first Component in the registry, or nullptr if none.
     */
	template<typename Component>
	inline const Component* frontValue() const
	{
		auto components = list<const Component>();
		return !components.empty() ? &components.front() : nullptr;
	}

	/**
	 * @brief Gets the first Component in the registry, if any, in a mutable context.
	 * @tparam Component the type of component to retrieve
	 * @return a pointer to the first Component in the registry, or nullptr if none.
	 */
	template<typename Component>
	inline Component* frontValue()
	{
		auto components = list<Component>();
		return !components.empty() ? &components.front() : nullptr;
	}

	/**
	 * @brief Gets the last Component in the registry, if any, in a const context.
	 * @tparam Component the type of component to retrieve
	 * @return a const pointer to the last Component in the registry, or nullptr if none.
	 */
	template<typename Component>
	inline const Component* backValue() const
	{
		auto components = list<const Component>();
		return !components.empty() ? &components.back() : nullptr;
	}

	/**
	 * @brief Gets the last Component in the registry, if any, in a mutable context.
	 * @tparam Component the type of component to retrieve
	 * @return a pointer to the last Component in the registry, or nullptr if none.
	 */
	template<typename Component>
	inline Component* backValue()
	{
		entt::entity lastEntity = _registry.view<Component>().back();
		return lastEntity != entt::null ? &_registry.get<Component>(lastEntity) : nullptr;
	}

	/**
	 * @brief Gets the index of id, based on its positioin in Components... registry.
	 * @tparam ...Components the set of Components to find ids position within
	 * @param id the entity id to find the index of
	 * @return the index of the entity, or -1 if it is not found.
	 */
	template<typename... Components>
	inline int index(entt::entity id) const
	{
		auto view = _registry.view<const Components...>();
		auto findIterator = std::find(view.begin(), view.end(), id);
		return findIterator != view.end() ? static_cast<int>(std::distance(view.begin(), findIterator)) : -1;
	}

	/**
	 * @brief Gets the entity in offset position from the front in the Components storage
	 * @tparam ...Components the type of components the entity should own
	 * @param offset the number of positions from the front of the registry to offset
	 * @return the entity in offset position, or entt::null if out of bounds
	*/
	template<typename... Components>
	inline entt::entity next(int offset) const
	{
		auto components = _registry.view<Components...>();
		return components.size() < offset && offset > 0 ? *std::next(components.begin(), offset) : entt::null;
	}

	/**
	 * @brief Gets the component in offset position from the front in the Components storage in a const context
	 * @tparam Component the type of components to retrieve
	 * @param offset the number of positions from the front of the registry to offset
	 * @return a const pointer to the  Component in offset position, or nullptr if out of bounds
	*/
	template<typename Component>
	inline const Component* nextValue(int offset) const
	{
		auto components = list<const Component>();
		return components.size() < offset && offset > 0 ? std::next(components.front(), offset) : nullptr;
	}

	/**
	 * @brief Gets the component in offset position from the front in the Components storage in a mutable context
	 * @tparam Component the type of components to retrieve
	 * @param offset the number of positions from the front of the registry to offset
	 * @return a pointer to the  Component in offset position, or nullptr if out of bounds
	*/
	template<typename Component>
	inline Component* nextValue(int offset)
	{
		auto components = list<Component>();
		return components.size() < offset && offset > 0 ? std::next(components.front(), offset) : nullptr;
	}

	/**
	 * @brief Creates a new entity and emplaces a new Component to it
	 * @tparam Component the type of Component to emplace
	 * @tparam ...ConstructorArgs the type of arguments to pass to the Component constructor
	 * @param ...args the arguments to be forward to the Component constructor
	 * @return A reference to the newly created Component
	*/
	template<typename Component, typename... ConstructorArgs>
	inline Component& createAndEmplace(ConstructorArgs&&... args)
	{
		return _registry.emplace<Component>(_registry.create(), std::forward<ConstructorArgs>(args)...);
	}

	template<typename Component>
	inline entt::entity insert(Component* component)
	{
		// there has got to be a better way to do this.
		std::vector<entt::entity> componentId(1);
		std::vector<Component*> components = { component };
		_registry.create(componentId.begin(), componentId.end());
		_registry.insert(componentId.begin(), componentId.end(), components);
		return componentId.front();
	}

	/**
	 * @brief Destroys the entity with a matching component instance.
	 * @tparam Component the component type to check for
	 * @param component the component instance to check
	 * @return true if the entity was destroyed, false otherwise
	 */
	template<typename Component>
	inline bool destroy(Component& component)
	{
		entt::entity id = find(&component);
		if (id == entt::null) { return false; }
		_registry.destroy(id);
		return true;
	}

	/**
	 * @brief Gets the id of the entity that component points to.
	 * @tparam Component The type of the component the entity should have
	 * @param component a pointer to the component
	 * @return the entity that component points to, or entt::null if not found.
	 */
	template<typename Component>
	entt::entity find(Component* component) const {
		auto viewEach = _registry.view<const Component>().each();
		auto matching = [component](const auto each) { return &std::get<1>(each) == component; };
		auto findResult = std::find_if(viewEach.begin(), viewEach.end(), matching);
		return findResult != viewEach.end() ? std::get<0>(*findResult) : entt::null;
	}

	/**
	 * @brief Finds the first entity that meets predicate
	 * @tparam ...Components the set of components the entity must contain
	 * @param predicate The predicate used to find the entity
	 * @return the first entity that meets the critieria, or entt::null if no matching enttity found.
	 */
	template<typename... Components>
	inline entt::entity find_if(std::predicate<entt::entity> auto predicate) const
	{
		auto view = _registry.view<Components...>();
		auto findIterator = std::ranges::find_if(view.begin(), view.end(), predicate);
		return findIterator != view.end() ? *findIterator : entt::null;
	}

	/**
	 * @brief Finds the first component that meets predicate within a const context
	 * @tparam Component the Components the entity must contain, and is returned.
	 * @param predicate The predicate used to find the entity
	 * @return a const pointer to the first component that meets the critieria, or nullptr if no matching component is found
	 */
	template<typename Component>
	inline const Component* findValue_if(std::predicate<const Component&> auto predicate) const
	{
		auto view = list<const Component>();
		auto findIterator = std::ranges::find_if(view, predicate);
		return findIterator != view.End() ? &(*findIterator) : nullptr;
	}

	/**
	 * @brief Finds the first component that meets predicate within a mutable context
	 * @tparam Component the Components the entity must contain, and is returned.
	 * @param predicate The predicate used to find the entity
	 * @return a pointer to the first component that meets the critieria, or nullptr if no matching component is found
	*/
	template<typename Component>
	inline Component* findValue_if(std::predicate<Component> auto predicate)
	{
		auto view = list<Component>();
		auto findIterator = std::ranges::find_if(view, predicate);
		return findIterator != view.end() ? &(*findIterator) : nullptr;
	}

	/**
	 * @brief Finds the first component that has a matching rule name in a const context
	 * @tparam Component the Components the entity must contain, and is returned.
				Must have a method getRule()->getType() which returns a string reference.
	 * @param ruleType the name of the rule to match
	 * @return A pointer to a const instance of the first Component with a matching name, or nullptr if no match is found
	*/
	template<typename Component>
	inline Component* findValueByName(const std::string& ruleType) const
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
	inline Component* findValueByName(const std::string& ruleType)
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
	inline int getEntityIndex(entt::entity entity)
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
	inline ReturnType totalBy(Funcs... funcs)
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
