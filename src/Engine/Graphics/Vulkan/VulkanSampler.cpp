#include "VulkanSampler.h"
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
#include "VulkanSampler.h"
#include "VulkanContext.h"

namespace OpenXcom
{

VulkanSampler::VulkanSampler(VulkanContext& context)
	: _context(context)
{
	// create the texture sampler
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo.magFilter = vk::Filter::eNearest;                    // Nearest-neighbor filtering for magnification (no blending)
	samplerInfo.minFilter = vk::Filter::eNearest;                    // Nearest-neighbor filtering for minification (no blending)
	samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge; // Clamp edges, no tiling
	samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge; // Clamp edges, no tiling
	samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge; // Only relevant for 3D textures
	samplerInfo.anisotropyEnable = VK_FALSE;                         // Disable anisotropic filtering
	samplerInfo.maxAnisotropy = 1.0f;                                // No anisotropy
	samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;      // Border color if clamping beyond edge
	samplerInfo.unnormalizedCoordinates = VK_FALSE;                  // Use normalized coordinates (0 to 1)
	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;        // No mipmap blending (uses base level only)
	samplerInfo.compareEnable = VK_FALSE;                            // No depth comparison
	samplerInfo.compareOp = vk::CompareOp::eAlways;                  // Not relevant since compare is disabled

	_sampler = _context.getDevice().createSampler(samplerInfo);
}

VulkanSampler::~VulkanSampler()
{
	std::function<void()> commandBufferFunc = [this]()
	{
		// Wait for the device to finish
		_context.getDevice().waitIdle();
	};
	
	VulkanQueueThread& transferQueueThread = _context.getTransferQueueThread();
	transferQueueThread.enqueueTask(commandBufferFunc).wait();

	_context.getDevice().destroySampler(_sampler);
}

VulkanSamplerFactory::VulkanSamplerFactory(VulkanContext& context)
	: _context(context)
{
	_sampler = std::make_unique<VulkanSampler>(context);
}

VulkanSamplerFactory::~VulkanSamplerFactory()
{
}

} // namespace OpenXcom
