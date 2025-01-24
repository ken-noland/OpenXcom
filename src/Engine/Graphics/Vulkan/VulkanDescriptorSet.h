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
#include <unordered_map>
#include <memory>

namespace OpenXcom
{

class VulkanContext;
class VulkanDescriptorPoolInfo;
class PipelineDefinition;

class VulkanDescriptorSet
{
	vk::DescriptorSet _descriptorSet;
	VulkanDescriptorPoolInfo* _poolInfo;

public:
	VulkanDescriptorSet(vk::DescriptorSet descriptorSet, VulkanDescriptorPoolInfo* poolInfo)
		: _descriptorSet(descriptorSet), _poolInfo(poolInfo) {}

	vk::DescriptorSet getDescriptorSet() const { return _descriptorSet; }

	VulkanDescriptorPoolInfo* getPoolInfo() const { return _poolInfo; }
};

struct VulkanDescriptorPoolSizes
{
	std::unordered_map<vk::DescriptorType, uint32_t> sizes = {
		{vk::DescriptorType::eSampler, 10},
		{vk::DescriptorType::eCombinedImageSampler, 10},
		{vk::DescriptorType::eSampledImage, 10},
		{vk::DescriptorType::eStorageImage, 10},
		{vk::DescriptorType::eUniformBuffer, 10},
		{vk::DescriptorType::eStorageBuffer, 10},
		{vk::DescriptorType::eUniformBufferDynamic, 10},
		{vk::DescriptorType::eStorageBufferDynamic, 10},
		{vk::DescriptorType::eInputAttachment, 10}};

	std::vector<vk::DescriptorPoolSize> toVkDescriptorPoolSizeVector(uint32_t multiplier = 1) const
	{
		std::vector<vk::DescriptorPoolSize> poolSizes;
		for (const auto& [type, count] : sizes)
		{
			poolSizes.push_back({type, count * multiplier});
		}
		return poolSizes;
	}
};

class VulkanDescriptorPoolInfo
{
protected:
	vk::DescriptorPool pool;
	uint32_t allocatedSets;
	uint32_t maxSets;

public:
	VulkanDescriptorPoolInfo(vk::DescriptorPool pool, uint32_t maxSets)
		: pool(pool), allocatedSets(0), maxSets(maxSets) {}

	vk::DescriptorPool getPool() const { return pool; }
	uint32_t getMaxSets() const { return maxSets; }
	bool hasCapacity() const { return allocatedSets < maxSets; }

	void incrementAllocatedSets() { allocatedSets++; }
	void decrementAllocatedSets() { allocatedSets--; }
};

class VulkanDescriptorPoolManager
{
	vk::Device _device;
	VulkanDescriptorPoolSizes _defaultPoolSizes;
	std::vector<std::unique_ptr<VulkanDescriptorPoolInfo>> _pools;
	uint32_t _poolMultiplier = 1;

public:
	VulkanDescriptorPoolManager(vk::Device device)
		: _device(device) {}

	~VulkanDescriptorPoolManager()
	{
		for (const auto& poolInfo : _pools)
		{
			_device.destroyDescriptorPool(poolInfo->getPool());
		}
	}

	std::unique_ptr<VulkanDescriptorSet> allocateDescriptorSet(const vk::DescriptorSetLayout& layout)
	{
		// Try to allocate from an existing pool
		for (auto& poolInfo : _pools)
		{
			if (poolInfo->hasCapacity())
			{
				auto result = tryAllocateDescriptorSet(poolInfo->getPool(), layout);
				if (result.first == vk::Result::eSuccess)
				{
					poolInfo->incrementAllocatedSets();
					return std::make_unique<VulkanDescriptorSet>(result.second, poolInfo.get());
				}
			}
		}

		// If all pools are exhausted, create a new one
		return createAndAllocateDescriptorSet(layout);
	}

	void deallocateDescriptorSet(const VulkanDescriptorSet& descriptorSet)
	{
		VulkanDescriptorPoolInfo* poolInfo = descriptorSet.getPoolInfo();
		vk::DescriptorSet vkSet = descriptorSet.getDescriptorSet();

		vk::Result result = _device.freeDescriptorSets(poolInfo->getPool(), 1, &vkSet);
		if (result == vk::Result::eSuccess)
		{
			poolInfo->decrementAllocatedSets();
		}
		else
		{
			throw std::runtime_error("Failed to deallocate descriptor set");
		}
	}

private:
	std::pair<vk::Result, vk::DescriptorSet> tryAllocateDescriptorSet(vk::DescriptorPool pool, const vk::DescriptorSetLayout& layout)
	{
		vk::DescriptorSetAllocateInfo allocInfo = {
			pool,
			1, &layout};

		vk::DescriptorSet descriptorSet;
		vk::Result result = _device.allocateDescriptorSets(&allocInfo, &descriptorSet);
		return {result, descriptorSet};
	}

	std::unique_ptr<VulkanDescriptorSet> createAndAllocateDescriptorSet(const vk::DescriptorSetLayout& layout)
	{
		auto newPool = createDescriptorPool();
		_pools.push_back(std::make_unique<VulkanDescriptorPoolInfo>(newPool.getPool(), newPool.getMaxSets()));

		auto result = tryAllocateDescriptorSet(newPool.getPool(), layout);
		if (result.first == vk::Result::eSuccess)
		{
			auto& poolInfo = _pools.back();
			poolInfo->incrementAllocatedSets();
			return std::make_unique<VulkanDescriptorSet>(result.second, poolInfo.get());
		}

		throw std::runtime_error("Failed to allocate descriptor set from newly created pool");
	}

	VulkanDescriptorPoolInfo createDescriptorPool()
	{
		auto poolSizes = _defaultPoolSizes.toVkDescriptorPoolSizeVector(_poolMultiplier);

		vk::DescriptorPoolCreateInfo poolInfo = {
			vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, // Allows freeing individual descriptor sets
			1000 * _poolMultiplier,                               // Max sets per pool
			static_cast<uint32_t>(poolSizes.size()), poolSizes.data()};

		vk::DescriptorPool pool = _device.createDescriptorPool(poolInfo);
		VulkanDescriptorPoolInfo poolInfoStruct(pool, 1000 * _poolMultiplier);

		_poolMultiplier *= 2; // Exponentially grow pool size for scalability
		return poolInfoStruct;
	}
};


class VulkanDescriptorSetFactory
{
protected:
	VulkanContext& _context;
	VulkanDescriptorPoolManager _poolManager;

public:
	VulkanDescriptorSetFactory(VulkanContext& context);
	~VulkanDescriptorSetFactory();

	vk::DescriptorSetLayout createDescriptorSetLayout(const PipelineDefinition& pipelineDefinition);
	void destroyDescriptorSetLayout(const vk::DescriptorSetLayout& layout);

	std::unique_ptr<VulkanDescriptorSet> createDescriptorSet(const vk::DescriptorSetLayout& layout)
	{
		return _poolManager.allocateDescriptorSet(layout);
	}

	void destroyDescriptorSet(const VulkanDescriptorSet& descriptorSet)
	{
		_poolManager.deallocateDescriptorSet(descriptorSet);
	}

};

} // namespace OpenXcom
