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
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace OpenXcom
{

template <typename ResourceType>
class ResourceManager
{
public:
	using Handle = uint32_t;
	static const Handle INVALID_HANDLE = static_cast<Handle>(-1);

protected:
	using ResourceTypePtr = std::unique_ptr<ResourceType>;
	using ResourceTypeRef = std::reference_wrapper<ResourceType>;
	using ResourceTypeRefOptional = std::optional<ResourceTypeRef>;
	using ResourceContainer = std::vector<ResourceTypePtr>;
	using HandleToIndexMap = std::unordered_map<Handle, std::size_t>;
	using IndexToHandleMap = std::vector<Handle>;

	ResourceContainer _resources;
	HandleToIndexMap _handleToIndex;
	IndexToHandleMap _indexToHandle;

	Handle nextHandle = 0;

public:
	ResourceManager() = default;
	virtual ~ResourceManager() = default;

	Handle add(ResourceTypePtr resource)
	{
		Handle handle = nextHandle++;
		size_t index = _resources.size();
		_resources.push_back(std::move(resource));
		_handleToIndex[handle] = index;
		_indexToHandle.push_back(handle); // Store handle for reverse lookup
		return handle;
	}

	ResourceTypeRefOptional get(Handle handle)
	{
		auto it = _handleToIndex.find(handle);
		if (it != _handleToIndex.end())
		{
			return _resources[it->second].get();
		}
		return std::nullopt; // Handle not found
	}

	void remove(Handle handle)
	{
		auto it = _handleToIndex.find(handle);
		if (it != _handleToIndex.end())
		{
			size_t index = it->second;

			// Move the last element to the 'index' position to keep vector compact
			_resources[index] = std::move(_resources.back());
			_resources.pop_back();

			// Update index tracking
			if (index < _resources.size())
			{
				Handle movedHandle = _indexToHandle.back();
				_handleToIndex[movedHandle] = index;
				_indexToHandle[index] = movedHandle;
			}

			// Remove the handle mapping
			_handleToIndex.erase(it);
			_indexToHandle.pop_back();
		}
	}
};

} // namespace OpenXcom