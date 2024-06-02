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
void toLua(lua_State* L, T arg)
{
	// Default implementation does nothing
}

template <typename Ret, typename... Args>
class LuaDispatchEvent : public LuaApi
{
	using LuaCallbackList = std::vector<LuaCallback>;

  private:
	LuaCallbackList _callbacks;

	template <typename T>
	inline void pushArgument(lua_State* L, T arg)
	{
		toLua(L, arg);
	}

	inline void pushArguments(lua_State* L) {}

	template <typename T, typename... Rest>
	inline void pushArguments(lua_State* L, T first, Rest... rest)
	{
		pushArgument(L, first);
		pushArguments(L, rest...);
	}

	static int registerCallback(lua_State* L)
	{
		return 0;
	}

  public:
	LuaDispatchEvent(const std::string& name) : LuaApi(name) {}

	virtual void onRegisterApi(lua_State* luaState, int parentTableIndex) override
	{
		createFunction(luaState, "registerCallback", registerCallback, this);
	}

	inline Ret dispatchCallback(Args... args)
	{
		for (const LuaCallback& callback : _callbacks)
		{
			lua_State* L = callback.L;
			lua_rawgeti(L, LUA_REGISTRYINDEX, callback.functionRef); // Get the function

			// Push the arguments onto the Lua stack
			pushArguments(L, args...);

			// Call the function with the arguments
			if (lua_pcall(L, sizeof...(Args), 1, 0) != LUA_OK)
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

	inline void registerCallback(const LuaCallback& callback)
	{
		_callbacks.push_back(callback);
	}


};

} // namespace Lua

} // namespace OpenXcom
