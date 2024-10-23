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
#include <entt/entt.hpp>
#include <simplerttr.h>

namespace OpenXcom
{

/// Function signature for getting a component from an entity.
using GetComponentFunc = void* (*)(entt::handle);

/// Name of the function to get a component from an entity.
const char* const GetComponentFuncName = "GetComponentFunc";

/// Function to allow access to get a raw pointer to a component from an entity to
///  be used by the RTTR library
template <typename ComponentType>
void* GetComponentRawPointer(entt::handle handle)
{
	return &handle.get<ComponentType>();
}

// Enum class used to specify how to serialize an object
enum class ObjectSerialize
{
	NEVER,	//default
	ALWAYS
};

// Enum class used to specify how to serialize a property
enum class PropertySerialize
{
	NEVER, // default
	ALWAYS
};

#define ENUM_VALUE_REGISTRATION(type, value) type::value, #value


} // OpenXcom