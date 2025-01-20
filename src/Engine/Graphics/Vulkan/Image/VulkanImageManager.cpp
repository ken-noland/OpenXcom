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

#include "VulkanImageManager.h"
#include "VulkanImage.h"
#include "../VulkanContext.h"

namespace OpenXcom
{

VulkanImageManager::VulkanImageManager(VulkanContext& context)
{
	_imageFactory = std::make_unique<VulkanImageFactory>(context);
}

std::unique_ptr<RenderTargetImage> VulkanImageManager::createRenderTarget(int width, int height, ImageFormat format)
{
	return _imageFactory->createRenderTarget(width, height, format);

	//// Create the game image
	//vk::ImageCreateInfo imageInfo{};
	//imageInfo.imageType = vk::ImageType::e2D;
	//imageInfo.extent = vk::Extent3D{width, height, 1}; // 320x200 resolution
	//imageInfo.mipLevels = 1;
	//imageInfo.arrayLayers = 1;
	//imageInfo.format = vk::Format::eR8G8B8A8Unorm; // 8-bit color with alpha
	//imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

	//_gameImage = _device.createImage(imageInfo);

	//vk::MemoryRequirements memRequirements = _device.getImageMemoryRequirements(_gameImage);

	//uint32_t memoryType = std::numeric_limits<uint32_t>::max();
	//vk::PhysicalDeviceMemoryProperties memProperties = _physicalDevice.getMemoryProperties();
	//for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	//{
	//	if ((memRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) == vk::MemoryPropertyFlagBits::eDeviceLocal)
	//	{
	//		memoryType = i;
	//		break;
	//	}
	//}

	//if (memoryType == std::numeric_limits<uint32_t>::max())
	//{
	//	throw std::runtime_error("Failed to find suitable memory type for game image!");
	//}

	//vk::MemoryAllocateInfo allocInfo{};
	//allocInfo.allocationSize = memRequirements.size;
	//allocInfo.memoryTypeIndex = memoryType;

	//_gameImageMemory = _device.allocateMemory(allocInfo);	// should I be using VMA here?

	//_device.bindImageMemory(_gameImage, _gameImageMemory, 0);

	//vk::ImageViewCreateInfo imageViewInfo{};
	//imageViewInfo.image = _gameImage;
	//imageViewInfo.viewType = vk::ImageViewType::e2D;
	//imageViewInfo.format = vk::Format::eR8G8B8A8Unorm;
	//imageViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	//imageViewInfo.subresourceRange.levelCount = 1;
	//imageViewInfo.subresourceRange.layerCount = 1;

	//_gameImageView = _device.createImageView(imageViewInfo);

	//vk::AttachmentDescription colorAttachment{};
	//colorAttachment.format = vk::Format::eR8G8B8A8Unorm;
	//colorAttachment.samples = vk::SampleCountFlagBits::e1;
	//colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	//colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	//colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	//colorAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	//vk::AttachmentReference colorAttachmentRef{};
	//colorAttachmentRef.attachment = 0; // Index of the attachment in the render pass (color attachment)
	//colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	//vk::SubpassDescription subpass{};
	//subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	//subpass.colorAttachmentCount = 1;
	//subpass.pColorAttachments = &colorAttachmentRef;

	//vk::RenderPassCreateInfo renderPassInfo{};
	//renderPassInfo.attachmentCount = 1;
	//renderPassInfo.pAttachments = &colorAttachment;
	//renderPassInfo.subpassCount = 1;
	//renderPassInfo.pSubpasses = &subpass;

	//_gameRenderPass = _device.createRenderPass(renderPassInfo);

	//vk::FramebufferCreateInfo framebufferInfo{};
	//framebufferInfo.renderPass = _gameRenderPass;
	//framebufferInfo.attachmentCount = 1;
	//framebufferInfo.pAttachments = &_gameImageView;
	//framebufferInfo.width = width;
	//framebufferInfo.height = height;
	//framebufferInfo.layers = 1;

	//_gameFramebuffer = _device.createFramebuffer(framebufferInfo);

	//// create the texture sampler
	//vk::SamplerCreateInfo samplerInfo{};
	//samplerInfo.magFilter = vk::Filter::eNearest;                    // Nearest-neighbor filtering for magnification (no blending)
	//samplerInfo.minFilter = vk::Filter::eNearest;                    // Nearest-neighbor filtering for minification (no blending)
	//samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge; // Clamp edges, no tiling
	//samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge; // Clamp edges, no tiling
	//samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge; // Only relevant for 3D textures
	//samplerInfo.anisotropyEnable = VK_FALSE;                         // Disable anisotropic filtering
	//samplerInfo.maxAnisotropy = 1.0f;                                // No anisotropy
	//samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;      // Border color if clamping beyond edge
	//samplerInfo.unnormalizedCoordinates = VK_FALSE;                  // Use normalized coordinates (0 to 1)
	//samplerInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;        // No mipmap blending (uses base level only)
	//samplerInfo.compareEnable = VK_FALSE;                            // No depth comparison
	//samplerInfo.compareOp = vk::CompareOp::eAlways;                  // Not relevant since compare is disabled

	//_textureSampler = _device.createSampler(samplerInfo);
	//return 0;
}

} // namespace OpenXcom
