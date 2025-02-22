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
#include <chrono>
#include <functional>

namespace OpenXcom
{

class RepeatAnimationTimer
{
protected:
	std::function<void()> _callback;                       // The function to call on each interval.
	std::chrono::milliseconds _interval;                   // Interval between each callback call.
	std::chrono::milliseconds _elapsedTime;                // Accumulated elapsed time.
	std::chrono::steady_clock::time_point _lastUpdateTime; // Last update time.
	bool _running;                                         // Indicates whether the timer is active.

public:
	// Constructor takes a callback function and an interval (in milliseconds) for the repeat.
	RepeatAnimationTimer(std::function<void()> callback, std::chrono::milliseconds interval)
		: _callback(callback), _interval(interval), _elapsedTime(0), _running(false) {}

	// Empty constructor.
	RepeatAnimationTimer()
		: _interval(0), _elapsedTime(0), _running(false) {}

	void setCallback(std::function<void()> callback)
	{
		_callback = callback;
	}

	void setInterval(std::chrono::milliseconds interval)
	{
		_interval = interval;
	}

	// Start (or restart) the timer.
	void start()
	{
		_running = true;
		_elapsedTime = std::chrono::milliseconds(0);
		_lastUpdateTime = std::chrono::steady_clock::now();
	}

	// Call this function regularly (e.g., every frame) to update the timer.
	void update()
	{
		if (!_running) return;

		auto now = std::chrono::steady_clock::now();
		auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastUpdateTime);
		_lastUpdateTime = now;
		_elapsedTime += deltaTime;

		// Process as many intervals as have elapsed.
		while (_elapsedTime >= _interval)
		{
			_callback(); // Execute the callback for this interval.
			_elapsedTime -= _interval;
		}
	}

	// Stop the timer.
	void stop()
	{
		_running = false;
	}

	// Check if the timer is currently running.
	bool isRunning() const
	{
		return _running;
	}

	// Reset the timer (does not start it).
	void reset()
	{
		_elapsedTime = std::chrono::milliseconds(0);
	}

};

} // namespace OpenXcom
