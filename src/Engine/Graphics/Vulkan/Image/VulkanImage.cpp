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
#include "VulkanImage.h"
#include "../VulkanContext.h"

namespace OpenXcom
{

VulkanImageFactory::VulkanImageFactory(VulkanContext& context)
	: _context(context)
{
}

VulkanImageFactory::~VulkanImageFactory()
{
}

VulkanImage::VulkanImage(VulkanContext& context, uint32_t width, uint32_t height, vk::ImageUsageFlags flags)
	: Image(ImageType::Texture), _context(context)
{
	throw new std::runtime_error("Not implemented");

	//_image = _context.getDevice().createImage(imageInfo);

	// vk::MemoryRequirements memRequirements = _device.getImageMemoryRequirements(_gameImage);

	// uint32_t memoryType = std::numeric_limits<uint32_t>::max();
	// vk::PhysicalDeviceMemoryProperties memProperties = _physicalDevice.getMemoryProperties();
	// for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	//{
	//	if ((memRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) == vk::MemoryPropertyFlagBits::eDeviceLocal)
	//	{
	//		memoryType = i;
	//		break;
	//	}
	// }

	// if (memoryType == std::numeric_limits<uint32_t>::max())
	//{
	//	throw std::runtime_error("Failed to find suitable memory type for game image!");
	// }

	// vk::MemoryAllocateInfo allocInfo{};
	// allocInfo.allocationSize = memRequirements.size;
	// allocInfo.memoryTypeIndex = memoryType;

	//_gameImageMemory = _device.allocateMemory(allocInfo);	// should I be using VMA here?

	//_device.bindImageMemory(_gameImage, _gameImageMemory, 0);


}

VulkanImage::~VulkanImage()
{
	vmaDestroyImage(_context.getAllocator(), _image, _allocation);
}


} // namespace OpenXcom
