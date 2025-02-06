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
#include <limits>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>
#include <functional>

namespace OpenXcom
{

template <typename ResourceType>
class ResourceManager;

template <typename ResourceType>
class ResourceHandle
{
protected:
	using HandleType = std::uint32_t;
	std::uint32_t _handle;

public:
	ResourceHandle() : _handle(0) {}
	ResourceHandle(HandleType handle) : _handle(handle) {}

	// Access the underlying handle
	std::uint32_t getId() const
	{
		return _handle;
	}

	// Comparison operators
	bool operator==(const ResourceHandle& other) const
	{
		return _handle == other._handle;
	}

	bool operator!=(const ResourceHandle& other) const
	{
		return _handle != other._handle;
	}

	// Less-than operator (useful for ordered containers like std::map)
	bool operator<(const ResourceHandle& other) const
	{
		return _handle < other._handle;
	}

	static const HandleType Invalid_Handle = std::numeric_limits<HandleType>::max();
};

template <typename ResourceType>
class OwningHandle
{
protected:
	ResourceHandle<ResourceType> _handle;          // Invalid handle by default
	ResourceManager<ResourceType>* _manager; // Pointer to the resource manager

public:
	// Blank constructor: Invalid handle
	OwningHandle() : _handle(ResourceHandle<ResourceType>::Invalid_Handle), _manager(nullptr) {}

	// Constructor: Takes ownership of the handle
	OwningHandle(std::uint32_t handle, ResourceManager<ResourceType>& manager)
		: _handle(handle), _manager(&manager) {}

	// Move constructor: Transfers ownership
	OwningHandle(OwningHandle&& other) noexcept
		: _handle(other._handle), _manager(other._manager)
	{
		other._handle = ResourceHandle<ResourceType>::Invalid_Handle; // Invalidate the old handle
	}

	// Move assignment: Transfers ownership
	OwningHandle& operator=(OwningHandle&& other) noexcept
	{
		if (this != &other)
		{
			release(); // Release current resource
			_handle = other._handle;
			_manager = other._manager;
			other._handle = 0; // Invalidate the old handle
			other._manager = nullptr;
		}
		return *this;
	}

	// Destructor: Releases the resource if valid
	~OwningHandle()
	{
		release();
	}

	// Delete copy constructor and copy assignment
	OwningHandle(const OwningHandle&) = delete;
	OwningHandle& operator=(const OwningHandle&) = delete;

	// Release the resource explicitly
	void release()
	{
		if (_handle != ResourceHandle<ResourceType>::Invalid_Handle && _manager != nullptr)
		{
			_manager->remove(_handle);
			_handle = ResourceHandle<ResourceType>::Invalid_Handle;
			_manager = nullptr;
		}
	}

	// Access the underlying id
	std::uint32_t getId() const
	{
		return _handle;
	}

	// Access the underlying resource
	ResourceType& get() const
	{
		return _manager->get(_handle);
	}

	// Dereference operator
	ResourceType& operator*() const
	{
		return get();
	}

	// Arrow operator
	ResourceType* operator->() const
	{
		return &get();
	}

	// Get a non-owning handle
	ResourceHandle<ResourceType> getHandle() const
	{
		return ResourceHandle<ResourceType>(_handle);
	}

	// Check if the handle is valid
	bool isValid() const
	{
		return _handle != ResourceHandle<ResourceType>::Invalid_Handle;
	}
};

} // namespace OpenXcom

namespace std
{

template <typename ResourceType>
struct hash<OpenXcom::ResourceHandle<ResourceType>>
{
	std::size_t operator()(const OpenXcom::ResourceHandle<ResourceType>& handle) const noexcept
	{
		return std::hash<std::uint32_t>{}(handle.getId());
	}
};

} // namespace std
