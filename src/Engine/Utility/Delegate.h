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
#include <list>
#include <functional>
#include <stdexcept>
#include <cassert>

namespace OpenXcom
{


// MulticastDelegate now uses composition (it has a vector member).
template <typename FunctionSignature>
class MulticastDelegate
{
public:
	using Function = std::function<FunctionSignature>;
	
    // Nested RAII type that owns the callback registration.
	class OwningFunctionHandle
	{
	public:
		// Default constructor: creates an empty handle.
		OwningFunctionHandle() noexcept
			: delegate_(nullptr), it_() {}

		// Disable copying.
		OwningFunctionHandle(const OwningFunctionHandle&) = delete;
		OwningFunctionHandle& operator=(const OwningFunctionHandle&) = delete;

		// Allow moving.
		OwningFunctionHandle(OwningFunctionHandle&& other) noexcept
			: delegate_(other.delegate_), it_(other.it_)
		{
			other.delegate_ = nullptr;
		}
		OwningFunctionHandle& operator=(OwningFunctionHandle&& other) noexcept
		{
			if (this != &other)
			{
				reset();
				delegate_ = other.delegate_;
				it_ = other.it_;
				other.delegate_ = nullptr;
			}
			return *this;
		}

		// Destructor automatically unregisters the callback.
		~OwningFunctionHandle()
		{
			reset();
		}

		// Optionally, allow manual release of the registration.
		void reset()
		{
			if (delegate_)
			{
				delegate_->remove(it_);
				delegate_ = nullptr;
			}
		}

	
    private:
		// Only MulticastDelegate can create a valid handle.
		friend class MulticastDelegate<FunctionSignature>;
		MulticastDelegate* delegate_ = nullptr;
		typename std::list<Function>::iterator it_;

		OwningFunctionHandle(MulticastDelegate* delegate, typename std::list<Function>::iterator it)
			: delegate_(delegate), it_(it)
		{
		}
	};
		
    // Convenience alias.
    using Handle = OwningFunctionHandle;

private:
	std::list<Function> functions;

public:
	MulticastDelegate() = default;
	~MulticastDelegate() {
		// Ensure all functions are removed before destruction.
		assert(functions.empty());
	}

	// Add a function to the delegate.
	// Returns a RAII handle that will remove the function on destruction.
	[[nodiscard]] Handle add(const Function& function)
	{
		functions.push_back(function);
		auto it = std::prev(functions.end());
		return Handle(this, it);
	}

	[[nodiscard]] Handle add(Function&& function)
	{
		functions.push_back(std::move(function));
		auto it = std::prev(functions.end());
		return Handle(this, it);
	}

	// Remove a function from the delegate given its iterator.
	void remove(typename std::list<Function>::iterator it)
	{
		functions.erase(it);
	}

	// Call all registered functions in order.
	template <typename... Args>
	void call(Args&&... args)
	{
		for (auto& func : functions)
		{
			func(std::forward<Args>(args)...);
		}
	}

	// Call all registered functions in reverse order.
	template <typename... Args>
	void rcall(Args&&... args)
	{
		for (auto it = functions.rbegin(); it != functions.rend(); ++it)
		{
			(*it)(std::forward<Args>(args)...);
		}
	}

};

} // namespace OpenXcom
