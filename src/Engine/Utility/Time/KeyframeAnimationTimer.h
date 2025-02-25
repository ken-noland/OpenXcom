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

#include <functional>
#include <chrono>
#include <vector>

namespace OpenXcom
{

// Structure to hold a single keyframe's data.
struct KeyframeAnimationFrame
{
	std::function<void()> callback;     // Function to call for this frame.
	std::chrono::milliseconds duration; // Duration to wait (since the last frame).
};

class KeyframeAnimationTimer
{
protected:
	TimeSystem& _time; // Reference to our high-precision TimeSystem.

	std::vector<KeyframeAnimationFrame> _frames; // List of animation frames.
	size_t _currentFrameIndex;                   // Current frame index.
	// Using a double-based millisecond duration for fractional precision.
	std::chrono::duration<double, std::milli> _elapsedTime; // Accumulated time.
	bool _running;                                          // Whether the timer is currently active.

public:
	// Constructor taking a TimeSystem reference and a vector of keyframe frames.
	KeyframeAnimationTimer(TimeSystem& time, const std::vector<KeyframeAnimationFrame>& frames)
		: _time(time),
		  _frames(frames),
		  _currentFrameIndex(0),
		  _elapsedTime(std::chrono::duration<double, std::milli>::zero()),
		  _running(false) {}

	// Empty constructor(still requires a time system).
	KeyframeAnimationTimer(TimeSystem& time)
		: _time(time),
		  _currentFrameIndex(0),
		  _elapsedTime(std::chrono::duration<double, std::milli>::zero()),
		  _running(false) {}

	// Set the frames.
	void setFrames(const std::vector<KeyframeAnimationFrame>& frames)
	{
		_frames = frames;
	}

	// Start (or restart) the timer.
	void start()
	{
		_running = true;
		_currentFrameIndex = 0;
		_elapsedTime = std::chrono::duration<double, std::milli>::zero();
	}

	// Update the timer; should be called once per frame.
	void update()
	{
		if (!_running || _currentFrameIndex >= _frames.size())
			return;

		// Retrieve delta time (in milliseconds) from the TimeSystem.
		std::chrono::duration<double, std::milli> dtMilli = _time.getDeltaTimeDuration();
		_elapsedTime += dtMilli;

		// Process all frames that are due based on the accumulated elapsed time.
		while (_currentFrameIndex < _frames.size() &&
			   _elapsedTime >= std::chrono::duration<double, std::milli>(_frames[_currentFrameIndex].duration.count()))
		{
			// Execute the callback for the current frame.
			_frames[_currentFrameIndex].callback();

			// Deduct the duration for this frame and move to the next frame.
			_elapsedTime -= std::chrono::duration<double, std::milli>(_frames[_currentFrameIndex].duration.count());
			++_currentFrameIndex;
		}

		// Stop the timer if all frames have been processed.
		if (_currentFrameIndex >= _frames.size())
		{
			_running = false;
		}
	}

	// Check if the timer is currently running.
	bool isRunning() const
	{
		return _running;
	}

	// Check if the animation has finished.
	bool isFinished() const
	{
		return !_running && _currentFrameIndex >= _frames.size();
	}

	// Reset the timer to allow the animation to run again.
	void reset()
	{
		_running = false;
		_currentFrameIndex = 0;
		_elapsedTime = std::chrono::duration<double, std::milli>::zero();
	}
};


} // namespace OpenXcom
