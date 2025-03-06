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
#include <memory>
#include <vector>

namespace OpenXcom
{

class State;
class GraphicsCommand;

/* Manages the game states.
 * 
 * The game states are updated and rendered in the order they were added.
 *
 * When a state is removed, it is not immediately deleted. Instead, it is
 * added to a list of states to be removed. The states to be removed are
 * then cleared up at the end of the update loop.
 */
class GameStates
{
protected:
	std::vector<std::unique_ptr<State>> _states;
	std::vector<State*> _toRemove;

	void cleanup()
	{
		for (State* stateToRemove : _toRemove)
		{
			auto it = std::find_if(_states.begin(), _states.end(),
								   [stateToRemove](const std::unique_ptr<State>& ptr) {
									   return ptr.get() == stateToRemove;
								   });
			if (it != _states.end())
			{
				// Remove the state.
				_states.erase(it);
			}
		}
		_toRemove.clear();
	}

public:
	GameStates() = default;
	~GameStates() { cleanup(); }

	void add(std::unique_ptr<State> state)
	{
		_states.push_back(std::move(state));
	}

	void remove(State& state)
	{
		_toRemove.push_back(&state);
	}

	void pop()
	{
		if (!_states.empty())
		{
			remove(*_states.back());
		}
	}

	void clear()
	{
		for(const std::unique_ptr<State>& state : _states)
		{
			remove(*state);
		}
	}

	void set(std::unique_ptr<State> state)
	{
		clear();
		add(std::move(state));
	}

	void update()
	{
		for (auto& state : _states)
		{
			state->onUpdate();
		}
		cleanup();
	}

	void render(GraphicsCommand& command)
	{
		for (auto& state : _states)
		{
			state->onRender(command);
		}
	}
};

} // namespace OpenXcom
