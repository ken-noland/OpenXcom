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
#include <memory>
#include <vulkan/vulkan.hpp>

namespace OpenXcom
{

// This whole header is mostly placeholder for now. There is one, and only one sampler at the moment and
//	I do want to have the ability to have future samplers, so I'm building out the manager and the
//	mechanism to support that.

class VulkanContext;

class VulkanSampler
{
protected:
	VulkanContext& _context;
	vk::Sampler _sampler;

public:
	VulkanSampler(VulkanContext& context /* TODO: Add parameters */);
	virtual ~VulkanSampler();

	vk::Sampler& getSampler() { return _sampler; }
};

class VulkanSamplerFactory
{
protected:
	VulkanContext& _context;

	//there's actually only one sampler for now
	std::unique_ptr<VulkanSampler> _sampler;

public:
	VulkanSamplerFactory(VulkanContext& context);
	virtual ~VulkanSamplerFactory();

	VulkanSampler& getSampler() { return *_sampler; }
};

};
