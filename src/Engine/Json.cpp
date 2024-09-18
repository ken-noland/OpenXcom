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
#include "Json.h"
#include <cassert>

namespace OpenXcom
{

template <>
bool fromJson<bool>(const nlohmann::json& json, bool& type)
{
	type = json.get<bool>();
	return true;
}

template <>
bool toJson<bool>(const bool& type, nlohmann::json& json)
{
	json = type;
	return true;
}

template <>
bool fromJson<char>(const nlohmann::json& json, char& type)
{
	type = json.get<char>();
	return true;
}

template <>
bool toJson<char>(const char& type, nlohmann::json& json)
{
	json = type;
	return true;
}

template <>
bool fromJson<unsigned char>(const nlohmann::json& json, unsigned char& type)
{
	type = json.get<unsigned char>();
	return true;
}

template <>
bool toJson<unsigned char>(const unsigned char& type, nlohmann::json& json)
{
	json = type;
	return true;
}


template <>
bool fromJson<short>(const nlohmann::json& json, short& type)
{
	type = json.get<short>();
	return true;
}

template <>
bool toJson<short>(const short& type, nlohmann::json& json)
{
	json = type;
	return true;
}

template <>
bool fromJson<unsigned short>(const nlohmann::json& json, unsigned short& type)
{
	type = json.get<unsigned short>();
	return true;
}

template <>
bool toJson<unsigned short>(const unsigned short& type, nlohmann::json& json)
{
	json = type;
	return true;
}

template <>
bool fromJson<int>(const nlohmann::json& json, int& type)
{
	type = json.get<int>();
	return true;
}

template <>
bool toJson<int>(const int& type, nlohmann::json& json)
{
	json = type;
	return true;
}

template <>
bool fromJson<unsigned int>(const nlohmann::json& json, unsigned int& type)
{
	type = json.get<unsigned int>();
	return true;
}

template <>
bool toJson<unsigned int>(const unsigned int& type, nlohmann::json& json)
{
	json = type;
	return true;
}

// std specializations

template <>
bool fromJson<std::string>(const nlohmann::json& json, std::string& type)
{
	type = json.get<std::string>();
	return true;
}

template <>
bool toJson<std::string>(const std::string& type, nlohmann::json& json)
{
	json = type;
	return true;
}

template <>
bool fromJson<std::filesystem::path>(const nlohmann::json& json, std::filesystem::path& type)
{
	type = json.get<std::filesystem::path>();
	return true;
}

template <>
bool toJson<std::filesystem::path>(const std::filesystem::path& type, nlohmann::json& json)
{
	json = type;
	return true;
}



}
