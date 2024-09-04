#pragma once
/*
 * Copyright 2010-2016 OpenXcom Developers.
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
#include "../../Engine/Registry.h"
#include "../../Engine/TypeErasedPtr.h"

#include <unordered_map>
#include <typeindex>

namespace OpenXcom
{


/// Entity Component System.
class ECS
{
protected:
	Registry _registry;

	using SystemRegistryContainer = std::unordered_map<std::type_index, TypeErasedUpdatePtr>;
	SystemRegistryContainer _systemRegistry;

	template <typename SystemType, typename... Args>
	inline SystemType& registerSystem(Args&&... args);

	using FactoryRegistryContainer = std::unordered_map<std::type_index, TypeErasedPtr>;
	FactoryRegistryContainer _factoryRegistry;

	template <typename FactoryType, typename... Args>
	inline FactoryType& registerFactory(Args&&... args);

public:
	ECS();
	~ECS();

	/// Returns the registry.
	Registry& getRegistry() { return _registry; };

	/// Return a system
	template <typename SystemType>
	inline SystemType& getSystem();

	/// Return a factory
	template <typename FactoryType>
	inline FactoryType& getFactory();

	/// Update all systems
	void update();
};

/// Register a system
template <typename SystemType, typename... Args>
inline SystemType& ECS::registerSystem(Args&&... args)
{
	TypeErasedUpdatePtr ptr(new SystemType(std::forward<Args>(args)...));
	auto [it, inserted] = _systemRegistry.emplace(std::type_index(typeid(SystemType)), std::move(ptr));
	return it->second.get<SystemType>();
}

/// Register a factory
template <typename FactoryType, typename... Args>
inline FactoryType& ECS::registerFactory(Args&&... args)
{
	TypeErasedPtr ptr = new FactoryType(std::forward<Args>(args)...);
	auto [it, inserted] = _factoryRegistry.emplace(std::type_index(typeid(FactoryType)), std::move(ptr));
	return it->second.get<FactoryType>();
}

/// Return a system
template <typename SystemType>
SystemType& ECS::getSystem()
{
	SystemRegistryContainer::iterator it = _systemRegistry.find(typeid(SystemType));
	if (it == _systemRegistry.end())
	{
		throw std::runtime_error("System not found");
	}
	return it->second.get<SystemType>();
}

/// Return a factory
template <typename FactoryType>
inline FactoryType& ECS::getFactory()
{
	FactoryRegistryContainer::iterator it = _factoryRegistry.find(typeid(FactoryType));
	if (it == _factoryRegistry.end())
	{
		throw std::runtime_error("System not found");
	}
	return it->second.get<FactoryType>();
}


///-----------------------------------------------------------------------------
/// Global functions

class Game;
extern Game* getGame();

/// Return a system
template <typename SystemType>
inline SystemType& getSystem()
{
	return getGame()->getECS().getSystem<SystemType>();
}

} // namespace OpenXcom
