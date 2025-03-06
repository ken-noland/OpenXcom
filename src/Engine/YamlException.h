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
#include "YamlContext.h"

namespace OpenXcom
{

class YamlException : public Exception
{
public:
	explicit YamlException(const ryml::ConstNodeRef& yaml, YamlContext& context, const std::string& message)
		: Exception([&]() {
			  const ryml::Location& location = context.getParser().location(yaml);
			  return std::format(
				  "YAML Error parsing node {} in file {} at line {}, col {}: {}",
				  yaml.has_key() ? yaml.key() : "<unknown>",
				  location.name,
				  location.line,
				  location.col,
				  message);
		  }()) {}

	static void throwIfNoValue(const ryml::ConstNodeRef& yaml, YamlContext& context)
	{
		if (!yaml.has_val()) { throw YamlException(yaml, context, "No value found."); }
	}
};

}
