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
#include "../VulkanCommand.h"
#include "../Image/VulkanImage.h"
#include "../Image/VulkanRenderTarget.h"
#include "../Buffer/VulkanBuffer.h"

namespace OpenXcom
{

VulkanPushConstant::VulkanPushConstant(ShaderStage stage, std::size_t size)
	: _stage(stage)
{
	_data.resize(size);
}

VulkanPushConstant::~VulkanPushConstant()
{
}

void VulkanPushConstant::copyTo(const void* data, std::size_t size)
{
	if (size != _data.size())
	{
		throw new std::runtime_error("Invalid size");
	}
	std::memcpy(_data.data(), data, size);
}


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
	if (_pipeline.getDescriptorSetLayout())
	{
		// create the descriptor set
		_descriptorSet = _context.getDescriptorSetFactory().createDescriptorSet(_pipeline.getDescriptorSetLayout());
	}

	// create the push constants
	for (const PushConstantDefinition& pushConstantDefinition : _pipeline.getPipelineDefinition().getResourceLayout().getPushConstants())
	{
		assert(!_pushConstants[(int)pushConstantDefinition.stage]);
		_pushConstants[(int)pushConstantDefinition.stage] = std::make_unique<VulkanPushConstant>(pushConstantDefinition.stage, pushConstantDefinition.type.type().size());
	}
}

void VulkanPipelineBinding::destroy()
{
}

vk::ShaderStageFlagBits VulkanPipelineBinding::getShaderStage(ShaderStage stage)
{
	switch (stage)
	{
	case ShaderStage::Vertex:
		return vk::ShaderStageFlagBits::eVertex;
	case ShaderStage::Fragment:
		return vk::ShaderStageFlagBits::eFragment;
	default:
		throw new std::runtime_error("Invalid shader stage");
	}
}

void VulkanPipelineBinding::setVertexBuffer(DeviceBuffer& buffer)
{
	_vertexBuffer = static_cast<VulkanDeviceBuffer&>(buffer);
}

void VulkanPipelineBinding::setIndexBuffer(DeviceBuffer& buffer)
{
	_indexBuffer = static_cast<VulkanDeviceBuffer&>(buffer);
}

void VulkanPipelineBinding::setUniformBuffer(ShaderStage stage, uint32_t binding, const DeviceBuffer& buffer)
{
	std::vector<std::optional<std::reference_wrapper<const VulkanDeviceBuffer>>>& uniformBuffers = _uniformBuffers[(int)stage];

	if (binding >= uniformBuffers.size())
	{
		uniformBuffers.resize(binding + 1);
	}

	vk::DescriptorType descriptorType;
	if(buffer.getUsage() == BufferUsage::Uniform)
	{
		descriptorType = vk::DescriptorType::eUniformBuffer;
	}
	else if (buffer.getUsage() == BufferUsage::Storage)
	{
		descriptorType = vk::DescriptorType::eStorageBuffer;
	}
	else
	{
		throw new std::runtime_error("Invalid buffer usage");
	}

	const VulkanDeviceBuffer& vulkanBuffer = static_cast<const VulkanDeviceBuffer&>(buffer);
	uniformBuffers[binding] = vulkanBuffer;

	vk::DescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = vulkanBuffer.getBuffer();	// The buffer handle
	bufferInfo.offset = 0;							// Offset within the buffer
	bufferInfo.range = vulkanBuffer.getSize();		// Size of the buffer being used

	vk::WriteDescriptorSet descriptorWrite{};
	descriptorWrite.dstSet = _descriptorSet->getDescriptorSet();          // Target descriptor set
	descriptorWrite.dstBinding = binding;                                 // Binding index in the shader
	descriptorWrite.dstArrayElement = 0;                                  // First array element to update
	descriptorWrite.descriptorType = descriptorType;                      // Uniform buffer
	descriptorWrite.descriptorCount = 1;                                  // Number of descriptors to update
	descriptorWrite.pBufferInfo = &bufferInfo;                            // Buffer info to bind

	_context.getDevice().updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
}

void VulkanPipelineBinding::setPushConstant(const SimpleRTTR::Type& type, ShaderStage stage, const void* data, std::size_t size)
{
	// Get the push constant
	VulkanPushConstant& pushConstant = *_pushConstants[(int)stage];
	// Copy the data to the push constant
	pushConstant.copyTo(data, size);
}

void VulkanPipelineBinding::setTexture(ShaderStage stage, uint32_t binding, const DeviceImage& image)
{
	// Get the sampler
	vk::Sampler sampler = _context.getSamplerFactory().getSampler().getSampler();

	// Get the image view and layout from the provided image
	vk::ImageView imageView;
	if(image.getType() == ImageType::Texture)
	{
		const VulkanDeviceImage& vulkanImage = static_cast<const VulkanDeviceImage&>(image);
		imageView = vulkanImage.getImageView();
	}
	else if (image.getType() == ImageType::RenderTarget)
	{
		const VulkanRenderTarget& renderTarget = static_cast<const VulkanRenderTarget&>(image);
		imageView = renderTarget.getImageView();
	}
	else
	{
		throw new std::runtime_error("Invalid image type");
	}

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

void VulkanPipelineBinding::commit(GraphicsCommand& command)
{
	vk::CommandBuffer& vkCommand = static_cast<VulkanCommand&>(command).getCommandBuffer();

	// bind the pipeline
	vkCommand.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline.getPipeline());

	// bind the descriptor set
	vk::DescriptorSet descriptorSets[] = {_descriptorSet->getDescriptorSet()};
	vkCommand.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipeline.getPipelineLayout(), 0, 1, descriptorSets, 0, nullptr);

	// push the push constants
	for (const std::unique_ptr<VulkanPushConstant>& pushConstant : _pushConstants)
	{
		if (pushConstant)
		{
			vkCommand.pushConstants(_pipeline.getPipelineLayout(), getShaderStage(pushConstant->_stage), 0, (uint32_t)pushConstant->_data.size(), pushConstant->_data.data());
		}
	}

	// Bind vertex and index buffers
	assert(_vertexBuffer.has_value() && "You must bind a vertex buffer to the pipeline binding prior to calling commit()");
	VulkanDeviceBuffer& vertexBuffer = _vertexBuffer.value().get();
	bool isIndexed = true;
	if (_indexBuffer.has_value())
	{
		VulkanDeviceBuffer& indexBuffer = _indexBuffer.value().get();
		vk::DeviceSize offsets[] = {0};
		vkCommand.bindVertexBuffers(0, vertexBuffer.getBuffer(), offsets);
		vkCommand.bindIndexBuffer(indexBuffer.getBuffer(), 0, vk::IndexType::eUint16);

		// finally, issue draw command
		uint32_t count = (uint32_t)(indexBuffer.getSize() / indexBuffer.getType().size());
		vkCommand.drawIndexed(count, 1, 0, 0, 0);
	}
	else
	{
		vk::DeviceSize offsets[] = {0};
		vkCommand.bindVertexBuffers(0, vertexBuffer.getBuffer(), offsets);

		// finally, issue draw command
		uint32_t count = (uint32_t)(vertexBuffer.getSize() / vertexBuffer.getType().size());
		vkCommand.draw(count, 1, 0, 0);
		isIndexed = false;
	}
}

} // namespace OpenXcom
