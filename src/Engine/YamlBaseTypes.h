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
#include <utf8.h>

namespace OpenXcom
{

// helper function to handle reading optional and required values
template <class Type, bool optional>
bool readYaml(const ryml::ConstNodeRef& yaml, const std::string& key, Type& object, YamlContext& context)
{
	if constexpr (!optional) // Required key: must exist.
	{
		return yaml.has_child(key.data()) ? fromYaml<Type>(yaml[key.c_str()], object, context)
										  : throw OpenXcom::YamlException(yaml, context, std::format("Missing required key: {}", key));
	}
	else // Optional key: check for existence.
	{
		if (yaml.has_child(key.data()))
		{
			// Look up the child node with the provided key.
			return fromYaml<Type>(yaml[key.c_str()], object, context);
		}
		// we're returning true because the key is optional, even though it wasn't found
		return true;
	}
}

template <>
bool fromYaml<bool>(ryml::ConstNodeRef const& yaml, bool& type, YamlContext& context)
{
	YamlException::throwIfNoValue(yaml, context);
	yaml >> type;
	return true;
}

template <>
bool toYaml<bool>(const bool& type, ryml::NodeRef& yaml)
{
	yaml << type;
	return true;
}

template <>
bool fromYaml<char>(ryml::ConstNodeRef const& yaml, char& type, YamlContext& context)
{
	YamlException::throwIfNoValue(yaml, context);
	yaml >> type;
	return true;
}

template <>
bool toYaml<char>(const char& type, ryml::NodeRef& yaml)
{
	yaml << type;
	return true;
}

template <>
bool fromYaml<unsigned char>(ryml::ConstNodeRef const& yaml, unsigned char& type, YamlContext& context)
{
	YamlException::throwIfNoValue(yaml, context);
	yaml >> type;
	return true;
}

template <>
bool toYaml<unsigned char>(const unsigned char& type, ryml::NodeRef& yaml)
{
	yaml << type;
	return true;
}

template <>
bool fromYaml<short>(ryml::ConstNodeRef const& yaml, short& type, YamlContext& context)
{
	YamlException::throwIfNoValue(yaml, context);
	yaml >> type;
	return true;
}

template <>
bool toYaml<short>(const short& type, ryml::NodeRef& yaml)
{
	yaml << type;
	return true;
}

template <>
bool fromYaml<unsigned short>(ryml::ConstNodeRef const& yaml, unsigned short& type, YamlContext& context)
{
	YamlException::throwIfNoValue(yaml, context);
	yaml >> type;
	return true;
}

template <>
bool toYaml<unsigned short>(const unsigned short& type, ryml::NodeRef& yaml)
{
	yaml << type;
	return true;
}

template <>
bool fromYaml<int>(ryml::ConstNodeRef const& yaml, int& type, YamlContext& context)
{
	YamlException::throwIfNoValue(yaml, context);
	yaml >> type;
	return true;
}

template <>
bool toYaml<int>(const int& type, ryml::NodeRef& yaml)
{
	yaml << type;
	return true;
}

template <>
bool fromYaml<unsigned int>(ryml::ConstNodeRef const& yaml, unsigned int& type, YamlContext& context)
{
	YamlException::throwIfNoValue(yaml, context);
	yaml >> type;
	return true;
}

template <>
bool toYaml<unsigned int>(const unsigned int& type, ryml::NodeRef& yaml)
{
	yaml << type;
	return true;
}

// std specializations

template <>
bool fromYaml<std::string>(ryml::ConstNodeRef const& yaml, std::string& type, YamlContext& context)
{
	YamlException::throwIfNoValue(yaml, context);
	ryml::csubstr str(yaml.val());
	type = std::string(str.begin(), str.end());
	return true;
}

template <>
bool toYaml<std::string>(const std::string& type, ryml::NodeRef& yaml)
{
	//	yaml << type;
	return false;
}

// template <>
// bool fromYaml<std::u16string>(ryml::ConstNodeRef const& yaml, std::u16string& type)
//{
//	assert(yaml.is_val());
//	yaml >> type;
//	return true;
// }
//
// template <>
// bool toYaml<std::u16string>(const std::u16string& type, ryml::NodeRef& yaml)
//{
//	yaml << type;
//	return true;
// }

template <>
bool fromYaml<std::u32string>(ryml::ConstNodeRef const& yaml, std::u32string& type, YamlContext& context)
{
	YamlException::throwIfNoValue(yaml, context);

	std::string str;
	fromYaml<std::string>(yaml, str, context);

	utf8::utf8to32(str.begin(), str.end(), std::back_inserter(type));

	return true;
}

// template <>
// bool toYaml<std::u32string>(const std::u32string& type, ryml::NodeRef& yaml)
//{
//	yaml << type;
//	return true;
// }

template <>
bool fromYaml<std::filesystem::path>(ryml::ConstNodeRef const& yaml, std::filesystem::path& type, YamlContext& context)
{
	YamlException::throwIfNoValue(yaml, context);

	std::string path;
	fromYaml<std::string>(yaml, path, context);
	type = path;

	return true;
}

// template <>
// bool toYaml<std::filesystem::path>(const std::filesystem::path& type, ryml::NodeRef& yaml)
//{
//	yaml << type.string();
//	return true;
// }



} // namespace OpenXcom
