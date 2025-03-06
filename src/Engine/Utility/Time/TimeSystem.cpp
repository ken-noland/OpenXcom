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
#include <thread>

namespace OpenXcom
{

void TimeSystem::update()
{
	// Calculate the minimum frame duration for 60 FPS.
	Duration minFrameDuration(1.0 / 60);

	// Get the current time.
	TimePoint currentTime = Clock::now();
	Duration frameDuration = currentTime - _lastTimePoint;

	// If the elapsed time is less than the minimum frame duration,
	// sleep for the remaining time.
	if (frameDuration < minFrameDuration)
	{
		auto sleepDuration = std::chrono::duration_cast<std::chrono::milliseconds>(minFrameDuration - frameDuration);
		std::this_thread::sleep_for(sleepDuration);

		// Update currentTime after sleeping.
		currentTime = Clock::now();
		frameDuration = currentTime - _lastTimePoint;
	}

	// Update the delta time and the last update time.
	_deltaTime = frameDuration;
	_lastTimePoint = currentTime;
}

} // namespace OpenXcom
