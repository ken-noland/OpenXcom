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

#include <format>

#include <ryml.hpp>

#include "Exception.h"

template <>
struct std::formatter<ryml::csubstr> : std::formatter<std::string_view>
{
	auto format(const ryml::csubstr& s, std::format_context& ctx) const
	{
		return std::formatter<std::string_view>::format(std::string_view(s.str, s.len), ctx);
	}
};

template <>
struct std::formatter<ryml::substr> : std::formatter<ryml::csubstr>
{
	auto format(const ryml::substr& s, std::format_context& ctx) const
	{
		return std::formatter<ryml::csubstr>::format(s, ctx);
	}
};

namespace OpenXcom
{

class YamlException : public Exception
{
public:
	explicit YamlException(const ryml::ConstNodeRef& yaml, const ryml::Parser& parser, const std::string& message)
		: Exception([&]() {
			  const ryml::Location& location = parser.location(yaml);
			  return std::format(
				  "YAML Error parsing node {} in file {} at line {}, col {}: {}",
				  yaml.key(),
				  location.name,
				  location.line,
				  location.col,
				  message);
		  }()) {}

	static void throwIfNoValue(const ryml::ConstNodeRef& yaml, const ryml::Parser& parser)
	{
		if (!yaml.has_val()) { throw YamlException(yaml, parser, "No value found."); }
	}
};

}
