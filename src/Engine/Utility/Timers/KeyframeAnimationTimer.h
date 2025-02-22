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
#include <functional>
#include <chrono>
#include <vector>

namespace OpenXcom
{

// Structure to hold a single keyframe's data
struct KeyframeAnimationFrame
{
	std::function<void()> callback;     // Function to call for this frame
	std::chrono::milliseconds duration; // Duration to wait (since the last frame)
};

class KeyframeAnimationTimer
{
protected:
	std::vector<KeyframeAnimationFrame> _frames;           // List of animation frames
	size_t _currentFrameIndex;                             // Current frame index
	std::chrono::milliseconds _elapsedTime;                // Accumulated time since last frame change
	std::chrono::steady_clock::time_point _lastUpdateTime; // Last time update() was called
	bool _running;                                         // Whether the timer is currently active

public:
	// Constructor takes a vector of keyframe frames
	KeyframeAnimationTimer(const std::vector<KeyframeAnimationFrame>& frames)
		: _frames(frames), _currentFrameIndex(0), _elapsedTime(0), _running(false) {}

	// Empty constructor
	KeyframeAnimationTimer()
		: _currentFrameIndex(0), _elapsedTime(0), _running(false) {}

	// Set the frames
	void setFrames(const std::vector<KeyframeAnimationFrame>& frames)
	{
		_frames = frames;
	}

	// Start (or restart) the timer
	void start()
	{
		_running = true;
		_currentFrameIndex = 0;
		_elapsedTime = std::chrono::milliseconds(0);
		_lastUpdateTime = std::chrono::steady_clock::now();
	}

	// Update the timer; should be called regularly (e.g., once per frame in your main loop)
	void update()
	{
		if (!_running || _currentFrameIndex >= _frames.size())
			return;

		// Calculate time elapsed since last update
		auto now = std::chrono::steady_clock::now();
		auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastUpdateTime);
		_lastUpdateTime = now;
		_elapsedTime += deltaTime;

		// Process all frames that are due based on the accumulated elapsed time
		while (_currentFrameIndex < _frames.size() &&
			   _elapsedTime >= _frames[_currentFrameIndex].duration)
		{
			// Execute the callback for the current frame
			_frames[_currentFrameIndex].callback();

			// Deduct the duration for this frame from the elapsed time and move to the next frame
			_elapsedTime -= _frames[_currentFrameIndex].duration;
			++_currentFrameIndex;
		}

		// Stop running if all frames have been processed
		if (_currentFrameIndex >= _frames.size())
		{
			_running = false;
		}
	}

	// Check if the timer is still running
	bool isRunning() const
	{
		return _running;
	}

	// Check if the animation has finished
	bool isFinished() const
	{
		return !_running && _currentFrameIndex >= _frames.size();
	}

	// Reset the timer to allow the animation to run again
	void reset()
	{
		_running = false;
		_currentFrameIndex = 0;
		_elapsedTime = std::chrono::milliseconds(0);
	}
};

} // namespace OpenXcom
