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

#include <ryml.hpp>

// this include is present for recursive inclusion in the inja templates.
#include "YamlException.h"

namespace OpenXcom
{

// forward declarations
template <typename ClassType>
bool fromYaml(ryml::ConstNodeRef const& yaml, ClassType& type, ryml::Parser const& parser);

template <typename ClassType>
bool toYaml(const ClassType& type, ryml::NodeRef& yaml);

// helper function to handle reading optional and required values
template <class Type, bool optional>
bool readYaml(const ryml::ConstNodeRef& yaml, const std::string& key, Type& object, const ryml::Parser& parser)
{
	if constexpr (!optional) // Required key: must exist.
	{
		return yaml.has_child(key.data()) ? fromYaml<Type>(yaml[key.c_str()], object, parser)
										  : throw OpenXcom::YamlException(yaml, parser, std::format("Missing required key: {}", key));
	}
	else // Optional key: check for existence.
	{
		if (yaml.has_child(key.data()))
		{
			// Look up the child node with the provided key.
			return fromYaml<Type>(yaml[key.c_str()], object, parser);
		}
		// we're returning true because the key is optional, even though it wasn't found
		return true;
	}
}

} // namespace OpenXcom
