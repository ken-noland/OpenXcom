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
#include "Timer.h"
#include "Game.h"
#include "Options.h"


#include <simplerttr.h>
SIMPLERTTR
{
	SimpleRTTR::registration().type<OpenXcom::ProgressTimerComponent>()
		.property(&OpenXcom::ProgressTimerComponent::_duration, "duration")
		.property(&OpenXcom::ProgressTimerComponent::_elapsed, "elapsed")
		.property(&OpenXcom::ProgressTimerComponent::_active, "active");
}

namespace OpenXcom
{

namespace
{

const uint32_t accurate = 4;
uint32_t slowTick()
{
	//static uint32_t old_time = SDL_GetTicks();
	//static Uint64 false_time = static_cast<Uint64>(old_time) << accurate;
	//Uint64 new_time = ((Uint64)SDL_GetTicks()) << accurate;
	//false_time += (new_time - old_time) / Timer::gameSlowSpeed;
	//old_time = (uint32_t)new_time;
	//return (uint32_t)(false_time >> accurate);
	return 0;
}

} // namespace

uint32_t Timer::gameSlowSpeed = 1;
int Timer::maxFrameSkip = 8; // this is a pretty good default at 60FPS.

/**
 * Initializes a new timer with a set interval.
 * @param interval Time interval in milliseconds.
 * @param frameSkipping Use frameskipping.
 */
Timer::Timer(uint32_t interval, bool frameSkipping) : _start(0), _frameSkipStart(0), _interval(interval), _running(false), _frameSkipping(frameSkipping)
{
	//Timer::maxFrameSkip = Options::maxFrameSkip;
}

/**
 *
 */
Timer::~Timer()
{
}

/**
 * Starts the timer running and counting time.
 */
void Timer::start()
{
	_frameSkipStart = _start = slowTick();
	_running = true;
}

/**
 * Stops the timer from running.
 */
void Timer::stop()
{
	_start = 0;
	_running = false;
}

/**
 * Returns the time passed since the last interval.
 * @return Time in milliseconds.
 */
uint32_t Timer::getTime() const
{
	if (_running)
	{
		return slowTick() - _start;
	}
	return 0;
}

/**
 * Returns if the timer has been started.
 * @return Running state.
 */
bool Timer::isRunning() const
{
	return _running;
}

/**
 * The timer keeps calculating the passed time while it's running,
 * calling the respective action handler whenever the set interval passes.
 * @param state State that the action handler belongs to.
 * @param surface Surface that the action handler belongs to.
 */
void Timer::think(bool state, bool surface)
{
	//Sint64 now = slowTick(); // must be signed to permit negative numbers
	//// assert(!game || game->isState(state));

	//if (_running)
	//{
	//	if ((now - _frameSkipStart) >= _interval)
	//	{
	//		for (int i = 0; i <= maxFrameSkip && isRunning() && (now - _frameSkipStart) >= _interval; ++i)
	//		{
	//			if (state && _stateFunction)
	//			{
	//				_stateFunction();
	//			}
	//			_frameSkipStart += _interval;
	//			// breaking here after one iteration effectively returns this function to its old functionality:
	//			if (!state || !_frameSkipping)
	//				break; // if game isn't set, we can't verify *state
	//		}

	//		if (surface && _surfaceFunction)
	//		{
	//			_surfaceFunction();
	//		}
	//		_start = slowTick();
	//		if (_start > _frameSkipStart)
	//			_frameSkipStart = _start; // don't play animations in ffwd to catch up :P
	//	}
	//}
}

/**
 * Changes the timer's interval to a new value.
 * @param interval Interval in milliseconds.
 */
void Timer::setInterval(uint32_t interval)
{
	_interval = interval;
}

/**
 * Sets a state function for the timer to call every interval.
 * @param handler Event handler.
 */
void Timer::onState(StateHandler handler)
{
	_stateFunction = handler;
}

/**
 * Sets a surface function for the timer to call every interval.
 * @param handler Event handler.
 */
void Timer::onSurface(SurfaceHandler handler)
{
	_surfaceFunction = handler;
}

TimeSystem::TimeSystem() : _lastTime(std::chrono::high_resolution_clock::now()), _frameTime(0), _elapsedTime(0)
{
}

/**
 * Updates the time system.
 */
void TimeSystem::update()
{
	std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
	_frameTime = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - _lastTime).count();
	_elapsedTime += _frameTime;
	_lastTime = currentTime;
}

ProgressTimerSystem::ProgressTimerSystem(entt::registry& registry, const TimeSystem& timeSystem)
	: _registry(registry), _timeSystem(timeSystem)
{
}

void ProgressTimerSystem::update()
{
	int64_t frameTime = _timeSystem.getFrameTimeMS();

	auto view = _registry.view<ProgressTimerComponent>();
	for (const entt::entity& entity : view)
	{
		ProgressTimerComponent& progressTimer = view.get<ProgressTimerComponent>(entity);
		if (progressTimer._active)
		{
			progressTimer._elapsed += frameTime;

			double progress = (double)progressTimer._elapsed / (double)progressTimer._duration;

			if (progress < 1.0)
			{
				callUpdate(entity, progressTimer, (float)progress);
			}

			if (progressTimer._elapsed >= progressTimer._duration)
			{
				progressTimer._elapsed = progressTimer._duration;
				progressTimer._active = false;
				if (progressTimer._onComplete)
				{
					callUpdate(entity, progressTimer, 1.0);
					callComplete(entity, progressTimer);
				}
			}
		}
	}
}

void ProgressTimerSystem::callUpdate(entt::entity entity, ProgressTimerComponent& component, float progress)
{
	if (component._onProgress)
	{
		component._onProgress(entt::handle(_registry, entity), progress);
	}
}

void ProgressTimerSystem::callComplete(entt::entity entity, ProgressTimerComponent& component)
{
	if (component._onComplete)
	{
		component._onComplete(entt::handle(_registry, entity));
	}
}

/**
 * Updates the time system.
 */
void IntervalTimerSystem::update()
{
}


}
