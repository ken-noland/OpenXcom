///*
// * Copyright 2010-2016 OpenXcom Developers.
// *
// * This file is part of OpenXcom.
// *
// * OpenXcom is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * OpenXcom is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
// */
#include "VulkanPipelineBinding.h"
#include "VulkanPipeline.h"
#include "../VulkanContext.h"
#include "../VulkanDescriptorSet.h"
#include "../VulkanSampler.h"
#include "../Image/VulkanImage.h"
#include "../VulkanRenderTarget.h"

namespace OpenXcom
{

VulkanPipelineBinding::VulkanPipelineBinding(VulkanContext& context, VulkanPipeline& pipeline)
	: _context(context), _pipeline(pipeline)
{
	create();
}

VulkanPipelineBinding::~VulkanPipelineBinding()
{
	destroy();
}

void VulkanPipelineBinding::create()
{
	// create the descriptor set
	_descriptorSet = _context.getDescriptorSetFactory().createDescriptorSet(_pipeline.getDescriptorSetLayout());
}

void VulkanPipelineBinding::destroy()
{
}

void VulkanPipelineBinding::recreate()
{
	destroy();
	create();
}

void VulkanPipelineBinding::setVertexBuffer(DeviceBuffer& buffer)
{
	_vertexBuffer = buffer;
}

void VulkanPipelineBinding::setIndexBuffer(DeviceBuffer& buffer)
{
	_indexBuffer = buffer;
}

void VulkanPipelineBinding::setPushConstant(SimpleRTTR::Type& type, ShaderStage stage, const void* data, std::size_t size)
{
}

void VulkanPipelineBinding::setTexture(ShaderStage stage, uint32_t binding, Image& image)
{
	// Get the sampler
	vk::Sampler sampler = _context.getSamplerFactory().getSampler().getSampler();

	// Get the image view and layout from the provided image
	vk::ImageView imageView;
	if(image.getType() == ImageType::Texture)
	{
		VulkanImage& vulkanImage = static_cast<VulkanImage&>(image);
		imageView = vulkanImage.getImageView();
	}
	else if (image.getType() == ImageType::RenderTarget)
	{
		VulkanRenderTarget& renderTarget = static_cast<VulkanRenderTarget&>(image);
		imageView = renderTarget.getImageView();
	}
	else
	{
		throw new std::runtime_error("Invalid image type");
	}

	vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal; // Adjust if needed

	// Create descriptor image info
	vk::DescriptorImageInfo descImageInfo{};

	descImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	descImageInfo.imageView = imageView;
	descImageInfo.sampler = sampler;

	// Create a write descriptor set
	vk::WriteDescriptorSet descriptorWrite{};

	descriptorWrite.dstSet = _descriptorSet->getDescriptorSet();
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &descImageInfo;

	// Update the descriptor set with the texture and sampler
	_context.getDevice().updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

	// TODO: Optionally, store the texture information for later use(recreating the pipeline binding if necessary)
	//_textures[binding] = &image;
}

} // namespace OpenXcom
