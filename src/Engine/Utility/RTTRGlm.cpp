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

// Run time type declarations for GLM types
#include "RTTR.h"

#include "../Yaml.h"
#include "../Json.h"
#include <glm/glm.hpp>
#include <charconv>

// Run time type information
SIMPLERTTR
{
	SimpleRTTR::registration().type<glm::vec2>()
		.meta("Name", "glm::vec2")							 // override the name deduction
		.meta("Serialize", OpenXcom::ObjectSerialize::NEVER) // we write our own serialization for this type
		.property(&glm::vec2::x, "x")
		.property(&glm::vec2::y, "y");

	SimpleRTTR::registration().type<glm::ivec2>()
		.meta("Name", "glm::ivec2")							 // override the name deduction
		.meta("Serialize", OpenXcom::ObjectSerialize::NEVER) // we write our own serialization for this type
		.property(&glm::ivec2::x, "x")
		.property(&glm::ivec2::y, "y");

	SimpleRTTR::registration().type<glm::vec3>()
		.meta("Name", "glm::vec3")							 // override the name deduction
		.meta("Serialize", OpenXcom::ObjectSerialize::NEVER) // we write our own serialization for this type
		.property(&glm::vec3::x, "x")
		.property(&glm::vec3::y, "y")
		.property(&glm::vec3::z, "z");
	
	SimpleRTTR::registration().type<glm::ivec3>()
		.meta("Name", "glm::ivec3")							 // override the name deduction
		.meta("Serialize", OpenXcom::ObjectSerialize::NEVER) // we write our own serialization for this type
		.property(&glm::ivec3::x, "x")
		.property(&glm::ivec3::y, "y")
		.property(&glm::ivec3::z, "z");
	
	SimpleRTTR::registration().type<glm::vec4>()
		.meta("Name", "glm::vec4")							 // override the name deduction
		.meta("Serialize", OpenXcom::ObjectSerialize::NEVER) // we write our own serialization for this type
		.property(&glm::vec4::x, "x")
		.property(&glm::vec4::y, "y")
		.property(&glm::vec4::z, "z")
		.property(&glm::vec4::w, "w");
	
	SimpleRTTR::registration().type<glm::ivec4>()
		.meta("Name", "glm::ivec4")							 // override the name deduction
		.meta("Serialize", OpenXcom::ObjectSerialize::NEVER) // we write our own serialization for this type
		.property(&glm::ivec4::x, "x")
		.property(&glm::ivec4::y, "y")
		.property(&glm::ivec4::z, "z")
		.property(&glm::ivec4::w, "w");

	SimpleRTTR::registration().type<glm::mat4>()
		.meta("Name", "glm::mat4")							  // override the name deduction
		.meta("Serialize", OpenXcom::ObjectSerialize::NEVER); // we write our own serialization for this type
}

namespace OpenXcom
{

//hack to get this to link
int FORCE_LINK_RTTRGLM = 0;

// glm::vec2
template <>
bool fromYaml<glm::vec2>(ryml::ConstNodeRef const& yaml, glm::vec2& type, YamlContext& context)
{
	return false;
}

template <>
bool toYaml<glm::vec2>(const glm::vec2& type, ryml::NodeRef& yaml)
{
	return false;
}

template <>
bool fromJson<glm::vec2>(const nlohmann::json& json, glm::vec2& type)
{
	return false;
}

template <>
bool toJson<glm::vec2>(const glm::vec2& type, nlohmann::json& json)
{
	return false;
}


// glm::ivec2
template <>
bool fromYaml<glm::ivec2>(ryml::ConstNodeRef const& yaml, glm::ivec2& type, YamlContext& context)
{
	return false;
}

template <>
bool toYaml<glm::ivec2>(const glm::ivec2& type, ryml::NodeRef& yaml)
{
	return false;
}

template <>
bool fromJson<glm::ivec2>(const nlohmann::json& json, glm::ivec2& type)
{
	return false;
}

template <>
bool toJson<glm::ivec2>(const glm::ivec2& type, nlohmann::json& json)
{
	return false;
}

// glm::vec3
template <>
bool fromYaml<glm::vec3>(ryml::ConstNodeRef const& yaml, glm::vec3& type, YamlContext& context)
{
	return false;
}

template <>
bool toYaml<glm::vec3>(const glm::vec3& type, ryml::NodeRef& yaml)
{
	return false;
}

template <>
bool fromJson<glm::vec3>(const nlohmann::json& json, glm::vec3& type)
{
	return false;
}

template <>
bool toJson<glm::vec3>(const glm::vec3& type, nlohmann::json& json)
{
	return false;
}


// glm::ivec3
template <>
bool fromYaml<glm::ivec3>(ryml::ConstNodeRef const& yaml, glm::ivec3& type, YamlContext& context)
{
	return false;
}

template <>
bool toYaml<glm::ivec3>(const glm::ivec3& type, ryml::NodeRef& yaml)
{
	return false;
}

template <>
bool fromJson<glm::ivec3>(const nlohmann::json& json, glm::ivec3& type)
{
	return false;
}

template <>
bool toJson<glm::ivec3>(const glm::ivec3& type, nlohmann::json& json)
{
	return false;
}


// glm::vec4
template <>
bool fromYaml<glm::vec4>(ryml::ConstNodeRef const& yaml, glm::vec4& type, YamlContext& context)
{
	return false;
}

template <>
bool toYaml<glm::vec4>(const glm::vec4& type, ryml::NodeRef& yaml)
{
	return false;
}

template <>
bool fromJson<glm::vec4>(const nlohmann::json& json, glm::vec4& type)
{
	return false;
}

template <>
bool toJson<glm::vec4>(const glm::vec4& type, nlohmann::json& json)
{
	return false;
}


// glm::ivec4
template <>
bool fromYaml<glm::ivec4>(ryml::ConstNodeRef const& yaml, glm::ivec4& type, YamlContext& context)
{
	if(yaml.is_seq())
	{
		// check that the number of elements is 4
		if(yaml.num_children() != 4)
		{
			throw YamlException(yaml, context, "Expected an array of 4 elements for glm::ivec4, but got " + std::to_string(yaml.num_children()));
		}

		for (std::size_t i = 0; i < 4; ++i)
		{
			c4::csubstr valueSubstr = yaml[i].val();
			std::string valueStr = std::string(valueSubstr.begin(), valueSubstr.end());
			int intValue;
			std::from_chars_result result = std::from_chars(valueStr.data(), valueStr.data() + valueStr.size(), intValue);
			if (result.ec != std::errc{})
			{
				throw YamlException(yaml, context, "Invalid integer value: " + valueStr);
			}
			type[i] = intValue;
		}

		return true;
	}
	else
	{
		throw YamlException(yaml, context, "Expected an array for glm::ivec4");
	}
}

template <>
bool toYaml<glm::ivec4>(const glm::ivec4& type, ryml::NodeRef& yaml)
{
	return false;
}

template <>
bool fromJson<glm::ivec4>(const nlohmann::json& json, glm::ivec4& type)
{
	return false;
}

template <>
bool toJson<glm::ivec4>(const glm::ivec4& type, nlohmann::json& json)
{
	return false;
}



// glm::mat4
template <>
bool fromYaml<glm::mat4>(ryml::ConstNodeRef const& yaml, glm::mat4& type, YamlContext& context)
{
	return false;
}

template <>
bool toYaml<glm::mat4>(const glm::mat4& type, ryml::NodeRef& yaml)
{
	return false;
}

template <>
bool fromJson<glm::mat4>(const nlohmann::json& json, glm::mat4& type)
{
	return false;
}

template <>
bool toJson<glm::mat4>(const glm::mat4& type, nlohmann::json& json)
{
	return false;
}


} // namespace OpenXcom
