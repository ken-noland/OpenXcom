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

namespace OpenXcom
{

template <typename ResourceType>
class ResourceManager
{
public:
	using Handle = ResourceHandle<ResourceType>;
	using OwningHandle = OwningHandle<ResourceType>;

protected:
	using ResourceMap = std::unordered_map<Handle, std::unique_ptr<ResourceType>>;

	ResourceMap _resources;
	std::uint32_t nextHandle = 0;

public:
	ResourceManager() = default;
	virtual ~ResourceManager() = default;

	OwningHandle add(std::unique_ptr<ResourceType> resource)
	{
		OwningHandle handle(nextHandle++, *this);
		_resources.emplace(handle.getHandle(), std::move(resource));
		return handle;
	}

	std::optional<std::reference_wrapper<ResourceType>> try_get(Handle handle)
	{
		auto it = _resources.find(handle);
		if (it != _resources.end())
		{
			return *it->second;
		}
		return std::nullopt;
	}

	template <typename Type = ResourceType>
	Type& get(const Handle& handle)
	{
		return *static_cast<Type*>(_resources[handle.getId()].get());
	}
		
	template <typename Type = ResourceType>
	Type& get(const OwningHandle& handle)
	{
		return get<Type>(handle.getHandle());
	}

	bool exists(const Handle& handle)
	{
		return _resources.find(handle) != _resources.end();
	}

	bool exists(const OwningHandle& handle)
	{
		return exists(handle.getHandle());
	}

	void remove(Handle handle)
	{
		_resources.erase(handle);
	}

	void remove(OwningHandle handle)
	{
		remove(handle.getHandle());
	}
};

} // namespace OpenXcom
