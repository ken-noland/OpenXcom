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
#include "State.h"
#include "Surface.h"

namespace OpenXcom
{

using StateHandler = std::function<void()>;
using SurfaceHandler = std::function<void()>;

/**
 * Timer used to run code in fixed intervals.
 * Used for code that should run at the same fixed interval
 * in various machines, based on milliseconds instead of CPU cycles.
 */
class Timer	//KN NOTE: this should be split into two timers. One for state and one for surface.
{
public:
	static int maxFrameSkip;
	static uint32_t gameSlowSpeed;

private:
	uint32_t _start;
	uint32_t _frameSkipStart;
	int _interval;
	bool _running;
	bool _frameSkipping;
	StateHandler _stateFunction;
	SurfaceHandler _surfaceFunction;

public:
	/// Creates a stopped timer.
	[[deprecated]] Timer(uint32_t interval, bool frameSkipping = false);
	/// Cleans up the timer.
	~Timer();
	/// Starts the timer.
	void start();
	/// Stops the timer.
	void stop();
	/// Gets the current time interval.
	uint32_t getTime() const;
	/// Gets if the timer's running.
	bool isRunning() const;
	/// Advances the timer.
	void think(bool state, bool surface);
	/// Sets the timer's interval.
	void setInterval(uint32_t interval);
	/// Hooks a state action handler to the timer interval.
	void onState(StateHandler handler);
	/// Hooks a surface action handler to the timer interval.
	void onSurface(SurfaceHandler handler);
};

/**
 * Time system updates once per frame and can be used to retrieve the duration of the last frame for purposes of animation.
 */
class TimeSystem
{
private:
	std::chrono::high_resolution_clock::time_point _lastTime;
	uint64_t _elapsedTime;
	uint64_t _frameTime;

public:
	TimeSystem();
	~TimeSystem() = default;

	void update();

	/**
	 * Gets the total elapsed time since the system started.
	 * @return Elapsed time in nanoseconds.
	 */
	inline uint64_t getElapsedTimeNS() const
	{
		return _elapsedTime;
	}

	/**
	 * Gets the time it took to render the last frame.
	 * @return Frame time in nanoseconds.
	 */
	inline uint64_t getFrameTimeNS() const
	{
		return _frameTime;
	}

	/**
	 * Gets the total elapsed time since the system started.
	 * @return Elapsed time in milliseconds.
	 */
	inline uint64_t getElapsedTimeMS() const
	{
		return getElapsedTimeNS() / 1000000;
	}

	/**
	 * Gets the time it took to render the last frame.
	 * @return Frame time in milliseconds.
	 */
	inline uint64_t getFrameTimeMS() const
	{
		return getFrameTimeNS() / 1000000;
	}

	/**
	 * Gets the time it took to render the last frame.
	 * @return Frame time in seconds.
	 */
	inline float getFrameTimeSeconds() const
	{
		return static_cast<float>(_frameTime) * 1e-9f; // Convert nanoseconds to seconds
	}
};


/**
 * Timer used to represent progress within a time space(duration).
 */
struct ProgressTimerComponent
{
	using ProgressCallback = std::function<void(entt::handle, double)>;
	using CompleteCallback = std::function<void(entt::handle)>;

	ProgressTimerComponent(uint64_t durationMS, ProgressCallback onProgress, CompleteCallback onComplete)
		: _duration(durationMS), _elapsed(0), _onProgress(onProgress), _onComplete(onComplete), _active(true) {}

	uint64_t _duration;                     // Total duration of the timer in milliseconds
	uint64_t _elapsed;                      // Elapsed time since the timer started in milliseconds

	bool _active;                           // Whether the timer is active

	ProgressCallback _onProgress;                  // Callback that receives the current progress (0 to 1)
	CompleteCallback _onComplete; // Callback when the timer reaches 100% (1.0)
};

/**
 * System for running progress timers
 */
class ProgressTimerSystem
{
protected:
	entt::registry& _registry;
	const TimeSystem& _timeSystem;

	void callUpdate(entt::entity entity, ProgressTimerComponent& component, float progress);
	void callComplete(entt::entity entity, ProgressTimerComponent& component);

public:
	ProgressTimerSystem(entt::registry& registry, const TimeSystem& timeSystem);
	~ProgressTimerSystem() = default;

	void update();
};

/**
 * Timer used to run code in fixed intervals.
 * Used for code that should run at the same fixed interval
 * in various machines, based on milliseconds
 */
struct IntervalTimerComponent
{
	uint32_t interval;
	uint32_t duration;
	uint32_t elapsed;
	uint32_t remaining;

	std::function<void(entt::handle)> onInterval;
	std::function<void(entt::handle)> onEnd;
};

/**
 * System for running interval timers.
 */
class IntervalTimerSystem
{
public:
	IntervalTimerSystem() = default;
	~IntervalTimerSystem() = default;

	void update();
};




}
