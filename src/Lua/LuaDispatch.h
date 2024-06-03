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
#include "LuaApi.h"

namespace YAML
{
class Node;
}

namespace OpenXcom
{

namespace Lua
{

struct LuaCallback
{
	lua_State* L;
	int functionRef;
};

template <typename T>
inline void toLuaArg(lua_State* L, T arg)
{
	// Default implementation does nothing
	//
	// if you get this error, you need to specialize the toLuaArg function for this type.
	static_assert(sizeof(T) == 0, "No toLuaArg function defined for this type");
}

// KN NOTE: temporarily placing these specializations here, until I find a better place for them
template <>
inline void toLuaArg(lua_State* L, int arg)
{
	lua_pushinteger(L, arg);
}

template <>
inline void toLuaArg(lua_State* L, float arg)
{
	lua_pushnumber(L, static_cast<lua_Number>(arg));
}

template <>
inline void toLuaArg(lua_State* L, double arg)
{
	lua_pushnumber(L, static_cast<lua_Number>(arg));
}

template <>
inline void toLuaArg(lua_State* L, const std::string& arg)
{
	lua_pushstring(L, arg.c_str());
}

template <>
inline void toLuaArg(lua_State* L, const char* arg)
{
	lua_pushstring(L, arg);
}

template <>
inline void toLuaArg(lua_State* L, const class YAML::Node& arg)
{
	// KN TODO: implement
}

template <typename T>
T&& fromLuaRet(lua_State* L)
{
//	// Default implementation does nothing
//	// 
//	// if you get this error, you need to specialize the fromLuaRet function for this type.
//	static_assert(sizeof(T) == 0, "No fromLuaRet function defined for this type");
}

template <typename Ret, typename... Args>
class LuaDispatchEvent : public LuaApi
{
	using LuaCallbackList = std::vector<LuaCallback>;

private:
	LuaCallbackList _callbacks;

	template <typename T>
	inline void pushArgument(lua_State* L, T&& arg)
	{
		toLuaArg<T>(L, std::forward<T>(arg));
	}

	inline void pushArguments(lua_State* L) {}

	template <typename T, typename... Rest>
	inline void pushArguments(lua_State* L, T&& first, Rest&&... rest)
	{
		pushArgument(L, std::forward<T>(first));
		pushArguments(L, std::forward<Rest>(rest)...);
	}

	inline void registerCallback(const LuaCallback& callback)
	{
		_callbacks.push_back(callback);
	}

	int registerCallback(lua_State* luaState)
	{
		if (lua_gettop(luaState) != 2 || !lua_isfunction(luaState, 2))
		{
			return luaL_error(luaState, "Expected a function as the argument");
		}

		// Store the function reference
		int functionRef = luaL_ref(luaState, LUA_REGISTRYINDEX);

		// Create the callback and store it
		registerCallback(LuaCallback{luaState, functionRef});

		return 0;
	}

	virtual void onRegisterApi(lua_State* luaState, int parentTableIndex) override
	{
		createClassFunction<LuaDispatchEvent, &LuaDispatchEvent::registerCallback>(luaState, "registerCallback");
	}

public:
	LuaDispatchEvent(const std::string& name) : LuaApi(name) {}

	inline Ret dispatchCallback(Args&&... args)
	{
		for (const LuaCallback& callback : _callbacks)
		{
			lua_State* L = callback.L;
			lua_rawgeti(L, LUA_REGISTRYINDEX, callback.functionRef); // Get the function

			// Push the arguments onto the Lua stack
			pushArguments(L, std::forward<Args>(args)...);

			// Call the function with the arguments
			if (lua_pcall(L, sizeof...(args), 1, 0) != LUA_OK)
			{
				Log(LOG_ERROR) << "Error calling Lua function: " << lua_tostring(L, -1) << std::endl;
				lua_pop(L, 1); // Pop the error message
			}
			else
			{
				// Handle the return value
				if (lua_isnumber(L, -1))
				{
					if constexpr (!std::is_void<Ret>::value)
					{
						// KN TODO: fromLua
						Ret result = static_cast<Ret>(lua_tonumber(L, -1));
						lua_pop(L, 1); // Pop the result
						return result; // For simplicity, return the result of the first successful callback
					}
					else
					{
						lua_pop(L, 1); // Pop the result
					}
					return;
				}
				lua_pop(L, 1); // Pop the result if not handled
			}
		}
		if constexpr (!std::is_void<Ret>::value)
		{
			return Ret(); // Return a default-constructed value if no callbacks are successful and Ret is not void
		}
	}



};

} // namespace Lua

} // namespace OpenXcom
