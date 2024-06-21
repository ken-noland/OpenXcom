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

#include <unordered_map>
#include <memory>
#include <typeindex>
#include <any>

namespace OpenXcom
{

/// Wrapper class for type erasure
class TypeErasedPtr
{
public:
	template <typename T>
	TypeErasedPtr(std::unique_ptr<T> ptr)
		: ptr_(ptr.release()), deleter_([](void* p) { delete static_cast<T*>(p); }) {}

	~TypeErasedPtr()
	{
		deleter_(ptr_);
	}

	template <typename T>
	T& get()
	{
		return *static_cast<T*>(ptr_);
	}

private:
	void* ptr_;
	std::function<void(void*)> deleter_;
};


/// Entity Component System.
class ECS
{
protected:
	Registry _registry;

	// It is worth noting that there are ways we could use an array instead of a map, with the
	// added benefit that we could save on a ton of memory, but at the moment I'm not sure if
	// it is worth it.

	using SystemRegistryContainer = std::unordered_map<std::type_index, TypeErasedPtr>;
	SystemRegistryContainer systemRegistry;

	template <typename SystemType, typename... Args>
	inline void registerSystem(Args&&... args);

public:
	ECS();
	~ECS();

	/// Returns the registry.
	Registry& getRegistry() { return _registry; };

	/// Return a system
	template <typename SystemType>
	inline SystemType& getSystem();
};

/// Register a system
template <typename SystemType, typename... Args>
inline void ECS::registerSystem(Args&&... args)
{
	systemRegistry.emplace(
		std::type_index(typeid(SystemType)),
		std::make_unique<SystemType>(std::forward<Args>(args)...));
}

/// Return a system
template <typename SystemType>
SystemType& ECS::getSystem()
{
	SystemRegistryContainer::iterator it = systemRegistry.find(typeid(SystemType));
	if (it == systemRegistry.end())
	{
		throw std::runtime_error("System not found");
	}
	return it->second.get<SystemType>();
}


///-----------------------------------------------------------------------------
/// Global functions

class Game;
extern Game* getGame();

/// Return a system
template <typename SystemType>
SystemType& getSystem()
{
	return getGame()->getECS().getSystem<SystemType>();
}

///// Return a factory
//template <typename FactoryType>
//FactoryType& getFactory()
//{
//	return getGame()->getECS().getFactory<FactoryType>();
//}

} // namespace OpenXcom
