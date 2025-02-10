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
#include "Handle.h"

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>
#include <limits>
#include <cassert>

namespace OpenXcom
{

template <typename ResourceType>
class ResourceManager
{
protected:
	// TODO: compact the map to an array for faster iterations
	std::unordered_map<ResourceHandle<ResourceType>, std::unique_ptr<ResourceType>> _resources;
	std::uint32_t nextHandle = 0;

public:
	ResourceManager() = default;
	virtual ~ResourceManager() = default;

	OwningHandle<ResourceType> add(std::unique_ptr<ResourceType> resource)
	{
		OwningHandle handle(nextHandle++, *this);
		_resources.emplace(handle.getHandle(), std::move(resource));
		return handle;
	}

	std::optional<std::reference_wrapper<ResourceType>> try_get(const ResourceHandle<ResourceType>& handle)
	{
		auto it = _resources.find(handle);
		if (it != _resources.end())
		{
			return *it->second;
		}
		return std::nullopt;
	}

	template <typename Type = ResourceType>
	Type& get(const ResourceHandle<ResourceType>& handle)
	{
		assert(handle != ResourceHandle<ResourceType>::Invalid_Handle);
		assert(exists(handle));
		return *static_cast<Type*>(_resources[handle.getId()].get());
	}
		
	template <typename Type = ResourceType>
	Type& get(const OwningHandle<ResourceType>& handle)
	{
		return get<Type>(handle.getHandle());
	}

	bool exists(const ResourceHandle<ResourceType>& handle)
	{
		return _resources.find(handle) != _resources.end();
	}

	bool exists(const OwningHandle<ResourceType>& handle)
	{
		return exists(handle.getHandle());
	}

	void remove(const ResourceHandle<ResourceType>& handle)
	{
		_resources.erase(handle);
	}

	void remove(const OwningHandle<ResourceType>& handle)
	{
		remove(handle.getHandle());
	}
};

} // namespace OpenXcom
