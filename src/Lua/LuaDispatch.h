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
#include "LuaArg.h"

namespace OpenXcom
{

namespace Lua
{

struct LuaCallbackData
{
	lua_State* luaState;
	int functionRef;

	bool operator==(const LuaCallbackData& other) const
	{
		return luaState == other.luaState && functionRef == other.functionRef;
	}
};

template <typename Ret, typename... Args>
class LuaCallback : public LuaApi
{
public:
	using LuaCallbackList = std::vector<LuaCallbackData>;

private:
	LuaCallbackList _callbacks;

protected:
	LuaCallback(const std::string& name) : LuaApi(name) {}

	LuaCallbackList& getCallbacks() { return _callbacks; }

	template <typename T>
	inline void pushArgument(lua_State* luaState, T&& arg)
	{
		toLuaArg<T>(luaState, std::forward<T>(arg));
	}

	inline void pushArguments(lua_State* luaState) {}

	template <typename T, typename... Rest>
	inline void pushArguments(lua_State* luaState, T&& first, Rest&&... rest)
	{
		pushArgument(luaState, std::forward<T>(first));
		pushArguments(luaState, std::forward<Rest>(rest)...);
	}

	inline void registerCallback(const LuaCallbackData& callback)
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
		registerCallback(LuaCallbackData{luaState, functionRef});

		return 0;
	}

	
	inline void unregisterCallback(const LuaCallbackData& callback)
	{
		_callbacks.erase(std::remove(_callbacks.begin(), _callbacks.end(), callback), _callbacks.end());
	}

	int unregisterCallback(lua_State* luaState)
	{
		if (lua_gettop(luaState) != 2 || !lua_isfunction(luaState, 2))
		{
			return luaL_error(luaState, "Expected a function as the argument");
		}

		// Store the function reference
		int functionRef = luaL_ref(luaState, LUA_REGISTRYINDEX);

		// Create the callback and store it
		unregisterCallback(LuaCallbackData{luaState, functionRef});

		return 0;
	}

	virtual void onRegisterApi(lua_State* luaState, int parentTableIndex) override
	{
		createClassFunction<LuaCallback, &LuaCallback::registerCallback>(luaState, "registerCallback");
		createClassFunction<LuaCallback, &LuaCallback::unregisterCallback>(luaState, "unregisterCallback");
	}
};

template <typename Ret, typename... Args>
class LuaSimpleCallback : public LuaCallback<Ret, Args...>
{
public:
	LuaSimpleCallback(const std::string& name) : LuaCallback(name) {}

	/// calls all registered callbacks with the given arguments
	inline Ret dispatchCallback(Args&&... args)
	{
		for (const LuaCallbackData& callback : getCallbacks())
		{
			lua_State* luaState = callback.luaState;
			lua_rawgeti(luaState, LUA_REGISTRYINDEX, callback.functionRef); // Get the function

			// Push the arguments onto the Lua stack
			pushArguments(luaState, std::forward<Args>(args)...);

			// Call the function with the arguments
			if (lua_pcall(luaState, sizeof...(args), 1, 0) != LUA_OK)
			{
				Log(LOG_ERROR) << "Error calling Lua function: " << lua_tostring(luaState, -1) << std::endl;
				lua_pop(luaState, 1); // Pop the error message
			}
			else
			{
				//check for a return and pop it off the stack
				while (lua_gettop(luaState) > 0) { lua_pop(luaState, 1); }	//KN NOTE: Maybe issue a warning?
			}
		}
		if constexpr (!std::is_void<Ret>::value)
		{
			return Ret(); // Return a default-constructed value if no callbacks are successful and Ret is not void
		}
	}
};

template <typename Ret, typename... Args>
class LuaAccumulatorCallback : public LuaCallback<Ret, Args...>
{
public:
	LuaAccumulatorCallback(const std::string& name) : LuaCallback(name) {}

	/// calls all registered callbacks with the given arguments, passing the results from one callback to the next and returning the result
	inline Ret dispatchCallback(Ret defaultValue, Args&&... args)
	{
		// make sure we are not using void as a return type
		static_assert(!std::is_void<Ret>::value, "LuaAccumulatorCallback cannot be used with void return type");

		// check that the first template argument is the same as the default value
		static_assert(std::is_same_v<Ret, std::decay_t<Ret>>, "The first template argument must be the same as the default value");

		Ret result = std::move(defaultValue);
		for (const LuaCallback& callback : getCallbacks())
		{
			lua_State* luaState = callback.luaState;
			lua_rawgeti(luaState, LUA_REGISTRYINDEX, callback.functionRef); // Get the function

			// Push the arguments onto the Lua stack
			pushArguments(luaState, std::forward<Args>(args)...);

			// Call the function with the arguments
			if (lua_pcall(luaState, sizeof...(args), 1, 0) != LUA_OK)
			{
				Log(LOG_ERROR) << "Error calling Lua function: " << lua_tostring(luaState, -1) << std::endl;
				lua_pop(luaState, 1); // Pop the error message
			}
			else
			{
				result = fromLuaArg<Ret>(L, -1);
			}
		}

		return result;
	}
};

//template <typename Ret, typename... Args>
//class LuaConditionalDispatchCallback : public LuaDispatch<Ret, Args...>
//{
//};

} // namespace Lua

} // namespace OpenXcom
