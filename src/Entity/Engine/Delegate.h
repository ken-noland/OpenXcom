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
#include <vector>
#include <functional>

namespace OpenXcom
{

template <typename FunctionSignature>
class MulticastDelegate : public std::vector<std::function<FunctionSignature>>
{
public:
	using Function = std::function<FunctionSignature>;

	/// Call all functions in the delegate.
	template <typename... Args>
	void call(Args&&... args)
	{
		for (Function& function : *this)
		{
			function(std::forward<Args>(args)...);
		}
	}

	/// Call all functions in reverse order.
	template <typename... Args>
	void rcall(Args&&... args)
	{
		for (auto it = this->rbegin(); it != this->rend(); ++it)
		{
			(*it)(std::forward<Args>(args)...);
		}
	}
};

} // namespace OpenXcom
