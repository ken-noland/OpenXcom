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
#include <vulkan/vulkan.hpp>
#include <thread>
#include <condition_variable>
#include <queue>
#include <functional>
#include <future>
#include <atomic>

namespace OpenXcom
{
	
class VulkanQueue
{
protected:
	vk::Queue _queue;
	uint32_t _familyIndex;

	vk::Device _device;
	
	vk::CommandPool _commandPool;
	std::vector<vk::CommandBuffer> _commandBuffers;

	friend class VulkanContext;
	void create(vk::Device device, uint32_t familyIndex, bool shouldCreateCommandBuffer);

public:
	VulkanQueue();
	~VulkanQueue();

	uint32_t getFamilyIndex() { return _familyIndex; }
	vk::Queue& getQueue() { return _queue; }

	vk::CommandPool& getCommandPool() { return _commandPool; }
	vk::CommandBuffer& getCommandBuffer(int index = 0) { return _commandBuffers[index]; }

	bool isValid() { return _familyIndex != std::numeric_limits<uint32_t>::max(); }
	void reset();
};

class VulkanQueueThread
{
protected:
    std::atomic<bool> running;
    std::thread workerThread;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::queue<std::function<void()>> taskQueue;

	void run();

public:
	VulkanQueueThread();
	~VulkanQueueThread();

	void start();
	void stop();

	std::future<void> enqueueTask(const std::function<void()>& task);
};

} // namespace OpenXcom
