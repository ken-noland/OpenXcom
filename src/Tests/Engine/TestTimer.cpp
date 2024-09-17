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
#include <gtest/gtest.h>

#include "../../Engine/Timer.h"

#include <chrono>
#include <thread>

using namespace OpenXcom;

// Example test
TEST(TimeSystemTest, update)
{
	const char* skip_slow_tests = std::getenv("SKIP_SLOW_TESTS");
	if (skip_slow_tests && std::string(skip_slow_tests) == "1")
	{
		GTEST_SKIP() << "Skipping slow tests based on SKIP_SLOW_TESTS environment variable.";
	}

	TimeSystem timeSystem;

	EXPECT_EQ(timeSystem.getElapsedTimeNS(), 0);
	EXPECT_EQ(timeSystem.getElapsedTimeMS(), 0);

	EXPECT_EQ(timeSystem.getFrameTimeNS(), 0);
	EXPECT_EQ(timeSystem.getFrameTimeMS(), 0);

	timeSystem.update();

	// I would expect the nanoseconds timer to go up by a small amount, but the millisecond
	// timer to stay at 0, but this is assuming you're running this on a fast enough system
	// that the time between the two calls to update is less than 1ms
	EXPECT_GT(timeSystem.getElapsedTimeNS(), 0);
	EXPECT_EQ(timeSystem.getElapsedTimeMS(), 0);

	EXPECT_GT(timeSystem.getFrameTimeNS(), 0);
	EXPECT_EQ(timeSystem.getFrameTimeMS(), 0);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	timeSystem.update();

	EXPECT_NEAR(timeSystem.getElapsedTimeNS(), 100000000, 20000000);
	EXPECT_NEAR(timeSystem.getElapsedTimeMS(), 100, 20);

	EXPECT_NEAR(timeSystem.getFrameTimeNS(), 100000000, 20000000);
	EXPECT_NEAR(timeSystem.getFrameTimeMS(), 100, 20);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	timeSystem.update();

	EXPECT_NEAR(timeSystem.getElapsedTimeNS(), 200000000, 40000000);
	EXPECT_NEAR(timeSystem.getElapsedTimeMS(), 200, 40);

	EXPECT_NEAR(timeSystem.getFrameTimeNS(), 100000000, 20000000);
	EXPECT_NEAR(timeSystem.getFrameTimeMS(), 100, 20);
}

TEST(ProgressTimerTest, ProgressCallbackCalledCorrectly)
{
	const char* skip_slow_tests = std::getenv("SKIP_SLOW_TESTS");
	if (skip_slow_tests && std::string(skip_slow_tests) == "1")
	{
		GTEST_SKIP() << "Skipping slow tests based on SKIP_SLOW_TESTS environment variable.";
	}

	bool onProgressCalled = false;
	double lastProgress = 0.0f;

	entt::registry registry;

	entt::entity entity = registry.create();
	entt::handle handle(registry, entity);

	ProgressTimerComponent::ProgressCallback progressFunc = [&](entt::handle, double progress)
	{
		onProgressCalled = true;
		lastProgress = progress;
	};

	ProgressTimerComponent::CompleteCallback completeFunc = [](entt::handle) {
	};

	handle.emplace<ProgressTimerComponent>(1000, progressFunc, completeFunc);

	TimeSystem timeSystem;
	ProgressTimerSystem progressTimerSystem(registry, timeSystem);

	timeSystem.update();
	progressTimerSystem.update();

	EXPECT_TRUE(onProgressCalled);
	EXPECT_NEAR(lastProgress, 0.0f, 0.01f);
	onProgressCalled = false;

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	timeSystem.update();
	progressTimerSystem.update();

	EXPECT_TRUE(onProgressCalled);
	EXPECT_NEAR(lastProgress, 0.10f, 0.05f);
}


TEST(ProgressTimerTest, CompleteCallbackCalledCorrectly)
{
	const char* skip_slow_tests = std::getenv("SKIP_SLOW_TESTS");
	if (skip_slow_tests && std::string(skip_slow_tests) == "1")
	{
		GTEST_SKIP() << "Skipping slow tests based on SKIP_SLOW_TESTS environment variable.";
	}

	bool onCompleteCalled = false;

	entt::registry registry;

	entt::entity entity = registry.create();
	entt::handle handle(registry, entity);

	ProgressTimerComponent::ProgressCallback progressFunc = [](entt::handle, double) {
	};

	ProgressTimerComponent::CompleteCallback completeFunc = [&](entt::handle)
	{
		onCompleteCalled = true;
	};

	ProgressTimerComponent& progressTimerComponent = handle.emplace<ProgressTimerComponent>(10, progressFunc, completeFunc);

	TimeSystem timeSystem;
	ProgressTimerSystem progressTimerSystem(registry, timeSystem);

	EXPECT_TRUE(progressTimerComponent._active);

	for (int i = 0; i < 20; i++)
	{
		timeSystem.update();
		progressTimerSystem.update();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	EXPECT_TRUE(onCompleteCalled);
	EXPECT_FALSE(progressTimerComponent._active);
}

TEST(ProgressTimerTest, ProgressDoesNotExceedOne)
{
	const char* skip_slow_tests = std::getenv("SKIP_SLOW_TESTS");
	if (skip_slow_tests && std::string(skip_slow_tests) == "1")
	{
		GTEST_SKIP() << "Skipping slow tests based on SKIP_SLOW_TESTS environment variable.";
	}

	bool onProgressCalled = false;
	double lastProgress = 0.0f;

	entt::registry registry;

	entt::entity entity = registry.create();
	entt::handle handle(registry, entity);

	ProgressTimerComponent::ProgressCallback progressFunc = [&](entt::handle, double progress)
	{
		onProgressCalled = true;
		lastProgress = progress;
	};

	ProgressTimerComponent::CompleteCallback completeFunc = [](entt::handle) {
	};

	handle.emplace<ProgressTimerComponent>(10, progressFunc, completeFunc);

	TimeSystem timeSystem;
	ProgressTimerSystem progressTimerSystem(registry, timeSystem);

	for (int i = 0; i < 20; i++)
	{
		timeSystem.update();
		progressTimerSystem.update();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	EXPECT_TRUE(onProgressCalled);
	EXPECT_NEAR(lastProgress, 1.0f, 0.01f);
}
