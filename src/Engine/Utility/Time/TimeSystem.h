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

namespace OpenXcom
{



class TimeSystem
{
public:
    // Define the clock type and related types for clarity.
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration  = std::chrono::duration<double>; // seconds precision as a double

protected:
	TimePoint _lastTimePoint; // Time point of the last update.
	Duration _deltaTime;      // Elapsed time between the current and last update.

public:
	TimeSystem() : _lastTimePoint(Clock::now()), _deltaTime(Duration::zero()) {};
	~TimeSystem() = default;

	void update();

	// Return delta time as a double in seconds
	double getDeltaTime() const
	{
		return _deltaTime.count();
	}

	// Alternatively, if you want to return a chrono duration:
	Duration getDeltaTimeDuration() const
	{
		return _deltaTime;
	}
};

} // namespace OpenXcom
