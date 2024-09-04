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
#include <functional>

namespace OpenXcom
{


/// Wrapper class for type erasure
class TypeErasedPtr
{
protected:
	void* _ptr;
	std::function<void(void*)> _deleter;

public:
	TypeErasedPtr() : _ptr(nullptr), _deleter(nullptr) {}

	template <typename T>
	TypeErasedPtr(T* ptr) : _ptr(ptr), _deleter([](void* p) { delete static_cast<T*>(p); }) {}

	TypeErasedPtr(void* ptr, std::function<void(void*)> deleter)
		: _ptr(ptr), _deleter(deleter) {}

	/// copy constructor
	TypeErasedPtr(const TypeErasedPtr& other) = delete;
	TypeErasedPtr& operator=(const TypeErasedPtr&) = delete;

	/// move constructor
	TypeErasedPtr(TypeErasedPtr&& other) noexcept
		: _ptr(std::exchange(other._ptr, nullptr)), _deleter(std::move(other._deleter))
	{
	}

	/// assignment move operator
	TypeErasedPtr& operator=(TypeErasedPtr&& other) noexcept
	{
		if (this != &other)
		{
			reset(); // Cleanup existing pointer
			_ptr = std::exchange(other._ptr, nullptr);
			_deleter = std::move(other._deleter);
		}
		return *this;
	}

	~TypeErasedPtr()
	{
		reset();
	}

	template <typename T>
	T& get()
	{
		return *static_cast<T*>(_ptr);
	}
		
	// Helper function to safely delete the managed pointer
	void reset()
	{
		if (_ptr)
		{
			_deleter(_ptr);
			_ptr = nullptr;
			_deleter = nullptr;
		}
	}

	bool valid() const
	{
		return _ptr != nullptr;
	}
};

// Derived TypeErasedUpdatePtr class with optional update function
class TypeErasedUpdatePtr : public TypeErasedPtr
{
protected:
	using UpdateFnPtr = void(*)(void*);
	UpdateFnPtr _updater;

	// SFINAE to detect if T has an update function
	template <typename T>
	static inline auto has_update(int) -> decltype(std::declval<T>().update(), std::true_type());

	template <typename T>
	static inline std::false_type has_update(...);

	template <typename T>
	void inline bind_update_function(std::true_type)
	{
		_updater = [](void* obj) { static_cast<T*>(obj)->update(); };
	}

	template <typename T>
	void inline bind_update_function(std::false_type)
	{
		_updater = nullptr;
	}

public:
	TypeErasedUpdatePtr() : TypeErasedPtr(), _updater(nullptr) {}

	template <typename T>
	TypeErasedUpdatePtr(T* ptr)
		: TypeErasedPtr(ptr, [](void* p)
						{ delete static_cast<T*>(p); })
	{
		bind_update_function<T>(has_update<T>(0));
	}

	~TypeErasedUpdatePtr()
	{
		reset();
	}

	/// copy constructor
	TypeErasedUpdatePtr(const TypeErasedUpdatePtr& other) = delete;
	TypeErasedUpdatePtr& operator=(const TypeErasedUpdatePtr&) = delete;

	/// move constructor
	TypeErasedUpdatePtr(TypeErasedUpdatePtr&& other) noexcept
		: TypeErasedPtr(std::exchange(other._ptr, nullptr), std::move(other._deleter)), _updater(std::move(other._updater)) {}

	/// assignment move operator
	TypeErasedUpdatePtr& operator=(TypeErasedUpdatePtr&& other) noexcept
	{
		if (this != &other)
		{
			reset(); // Cleanup existing pointer
			_ptr = std::exchange(other._ptr, nullptr);
			_deleter = std::move(other._deleter);
			_updater = std::move(other._updater);
		}
		return *this;
	}

	void update()
	{
		_updater(_ptr); // Call the stored function pointer with the raw object pointer
	}

	bool hasUpdate() const
	{
		return _updater != nullptr;
	}

	// Helper function to safely delete the managed pointer
	void reset()
	{
		TypeErasedPtr::reset();
		_updater = nullptr;
	}
};

// Definitions for the SFINAE methods outside the class to avoid linker errors
template <typename T>
inline auto TypeErasedUpdatePtr::has_update(int) -> decltype(std::declval<T>().update(), std::true_type())
{
	return std::true_type();
}

template <typename T>
inline std::false_type TypeErasedUpdatePtr::has_update(...)
{
	return std::false_type();
}

}
