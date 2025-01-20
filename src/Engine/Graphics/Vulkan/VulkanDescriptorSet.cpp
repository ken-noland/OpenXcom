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
#include "VulkanDescriptorSet.h"
#include "VulkanContext.h"
#include "../../Resource/Pipeline/PipelineDefinition.h"

namespace OpenXcom
{

VulkanDescriptorSetFactory::VulkanDescriptorSetFactory(VulkanContext& context)
	: _context(context), _poolManager(context.getDevice())
{
}

VulkanDescriptorSetFactory::~VulkanDescriptorSetFactory()
{
}

vk::DescriptorSetLayout VulkanDescriptorSetFactory::createDescriptorSetLayout(const PipelineDefinition& pipelineDefinition)
{
	std::vector<vk::DescriptorSetLayoutBinding> bindings;
		
	// Combined image samplers
	for (const CombinedImageSamplerDefinition& combinedImageSampler : pipelineDefinition.getResourceLayout().getCombinedImageSamplers())
	{
		vk::DescriptorSetLayoutBinding binding = {
			combinedImageSampler.binding,
			vk::DescriptorType::eCombinedImageSampler,
			1,
			vk::ShaderStageFlagBits::eFragment,
			nullptr};
		bindings.push_back(binding);
	}

	// Uniform buffers
	for (const UniformBufferDefinition& uniformBuffer : pipelineDefinition.getResourceLayout().getUniformBuffers())
	{
		vk::DescriptorSetLayoutBinding binding = {
			uniformBuffer.binding,
			vk::DescriptorType::eUniformBuffer,
			1,
			vk::ShaderStageFlagBits::eVertex,
			nullptr};
		bindings.push_back(binding);
	}

	vk::DescriptorSetLayoutCreateInfo layoutInfo = {
		vk::DescriptorSetLayoutCreateFlags(),
		static_cast<uint32_t>(bindings.size()), bindings.data()};

	return _context.getDevice().createDescriptorSetLayout(layoutInfo);
}

void VulkanDescriptorSetFactory::destroyDescriptorSetLayout(const vk::DescriptorSetLayout& layout)
{
	_context.getDevice().destroyDescriptorSetLayout(layout);
}

} // namespace OpenXcom
