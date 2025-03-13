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
#include "VulkanQueue.h"
#include "../../Logger.h"

namespace OpenXcom
{

VulkanQueue::VulkanQueue()
	: _device(nullptr), _familyIndex(std::numeric_limits<uint32_t>::max()), _queue(nullptr), _commandPool(nullptr)
{
}

VulkanQueue::~VulkanQueue()
{
	reset();
}

void VulkanQueue::reset()
{
	if (_device)
	{
		if (_commandBuffers.size() > 0)
		{
			_device.freeCommandBuffers(_commandPool, _commandBuffers);
			_commandBuffers.clear();
		}

		if (_commandPool)
		{
			_device.destroyCommandPool(_commandPool);
			_commandPool = nullptr;
		}
		_device = nullptr;
	}

	_familyIndex = std::numeric_limits<uint32_t>::max();
	_queue = nullptr;
}

void VulkanQueue::create(vk::Device device, uint32_t familyIndex, bool shouldCreateCommandBuffer)
{
	_device = device;
	_familyIndex = familyIndex;

	_queue = _device.getQueue(_familyIndex, 0);

	if(shouldCreateCommandBuffer)
	{
		// create the command pool
		vk::CommandPoolCreateInfo createInfo;
		createInfo.queueFamilyIndex = _familyIndex;
		createInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;	//vk::CommandPoolCreateFlagBits::eTransient?
		_commandPool = _device.createCommandPool(createInfo);

		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandPool = _commandPool;

		//If/when we ever have multithreaded rendering, it would be good to have command buffers for each thread.
		allocInfo.commandBufferCount = 1;

		_commandBuffers = _device.allocateCommandBuffers(allocInfo);
	}
}

VulkanQueueThread::VulkanQueueThread()
	: running(false)
{
}

VulkanQueueThread::~VulkanQueueThread()
{
}

void VulkanQueueThread::start()
{
	running = true;
	workerThread = std::thread(&VulkanQueueThread::run, this);
}

void VulkanQueueThread::stop()
{
	{	// scope for lock_guard
		std::lock_guard<std::mutex> lock(queueMutex);
		running = false;
	}

	cv.notify_one();

	if (workerThread.joinable()) {
		workerThread.join();
	}
}

void VulkanQueueThread::run()
{
	while (true)
	{
		std::function<void()> task;
		{
			std::unique_lock<std::mutex> lock(queueMutex);

			// Wait until there is a task or the shutdown signal.
			cv.wait(lock, [this] { return !taskQueue.empty() || !running; });

			// If shutdown is signaled and no more tasks remain, exit.
			if (!running && taskQueue.empty()) {
				break;
			}

			// Get the next task.
			task = std::move(taskQueue.front());
			taskQueue.pop();
		}

		// Execute the task (this could be your Vulkan transfer command).
		if (task) {
			task();
		}
	}
}

// Enqueue a task (e.g., a Vulkan transfer command) to be executed by the transfer thread.
std::future<void> VulkanQueueThread::enqueueTask(const std::function<void()>& task)
{
	// Create a promise and get its future.
	std::shared_ptr<std::promise<void>> promise = std::make_shared<std::promise<void>>();
	std::future<void> future = promise->get_future();

	// Wrap the original task with a lambda that sets the promise once done.
	std::function<void()> wrappedTask = [promise, task]() mutable {
		try {
			task();
			promise->set_value();
		} catch (...) {
			// In case the task throws, propagate the exception to the future.
			promise->set_exception(std::current_exception());
		}
	};

	{
		std::lock_guard<std::mutex> lock(queueMutex);
		taskQueue.push(wrappedTask);
	}
	cv.notify_one();
	return future;
}

	
} // namespace OpenXcom
