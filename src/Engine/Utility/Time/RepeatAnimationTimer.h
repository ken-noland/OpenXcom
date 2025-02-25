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
#include "TimeSystem.h"

#include <chrono>
#include <functional>

namespace OpenXcom
{

class RepeatAnimationTimer
{
protected:
	TimeSystem& _time; // Reference to our high-precision time system.

	std::function<void()> _callback; // The function to call on each interval.

	// Use duration<double, milli> to preserve sub-millisecond precision.
	std::chrono::duration<double, std::milli> _interval;    // Interval between each callback.
	std::chrono::duration<double, std::milli> _elapsedTime; // Accumulated elapsed time.

	bool _running; // Indicates whether the timer is active.

public:
	// Constructor that accepts a TimeSystem reference, a callback, and an interval.
	RepeatAnimationTimer(TimeSystem& time,
						 std::function<void()> callback,
						 std::chrono::duration<double, std::milli> interval)
		: _time(time), _callback(callback), _interval(interval),
		  _elapsedTime(std::chrono::duration<double, std::milli>::zero()),
		  _running(false) {}

	// Empty constructor.
	RepeatAnimationTimer(TimeSystem& time)
		: _time(time),
		  _interval(std::chrono::duration<double, std::milli>::zero()),
		  _elapsedTime(std::chrono::duration<double, std::milli>::zero()),
		  _running(false) {}

	void setCallback(std::function<void()> callback)
	{
		_callback = callback;
	}

	void setInterval(std::chrono::duration<double, std::milli> interval)
	{
		_interval = interval;
	}

	// Start (or restart) the timer.
	void start()
	{
		_running = true;
		_elapsedTime = std::chrono::duration<double, std::milli>::zero();
	}

	// Call this function regularly (e.g., every frame) to update the timer.
	void update()
	{
		if (!_running)
			return;

		// Retrieve delta time (in milliseconds) from the TimeSystem.
		std::chrono::duration<double, std::milli> dtMilli = _time.getDeltaTimeDuration();

		_elapsedTime += dtMilli;

		// Process as many intervals as have elapsed.
		while (_elapsedTime >= _interval)
		{
			if (_callback)
			{
				_callback(); // Execute the callback for this interval.
			}
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
		_elapsedTime = std::chrono::duration<double, std::milli>::zero();
	}
};

} // namespace OpenXcom
