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
#include <sstream>

namespace OpenXcom
{

// A lightweight assertion handler that collects an error message via operator<<.
class YamlAssertHandler
{
private:
	bool _active;
	const char* _expr;
	const char* _file;
	int _line;
	std::ostringstream _ss;

public:
	// If 'active' is true, the assertion failed.
	YamlAssertHandler(bool active, const char* expr, const char* file, int line)
		: _active(active), _expr(expr), _file(file), _line(line) {}

	// When the handler is destroyed, if it is active, print the message and abort.
	~YamlAssertHandler();

	// Allow streaming additional message text.
	template <typename T>
	YamlAssertHandler& operator<<(const T& value)
	{
		if (_active)
		{
			_ss << value;
		}
		return *this;
	}
};

#define YAML_ASSERT_MSG(condition) \
	YamlAssertHandler(!(condition), #condition, __FILE__, __LINE__)


} // namespace OpenXcom
