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

#include "../../../Resource/Pipeline/Pipeline.h"
#include "../../../Resource/Pipeline/PipelineDefinition.h"
#include <vulkan/vulkan.hpp>

namespace OpenXcom
{

class VulkanContext;
class VulkanSurface;
class VulkanDescriptorSetFactory;

class VulkanPipeline : public Pipeline
{
protected:
	VulkanContext& _context;

	PipelineDefinition _pipelineDefinition;

	vk::Pipeline _pipeline;
	vk::PipelineLayout _pipelineLayout;
	vk::DescriptorSetLayout _descriptorSetLayout;


	//--
	// helper functions
	void createDescriptorSetLayout();
	std::array<vk::PipelineShaderStageCreateInfo, 2> createShaderStages();

	void createVertexInputInfo(vk::VertexInputBindingDescription& bindingDescription, std::vector<vk::VertexInputAttributeDescription>& attributeDescriptions, vk::PipelineVertexInputStateCreateInfo& vertexInputInfo);
	void createInputAssemblyState(vk::PipelineInputAssemblyStateCreateInfo& inputAssembly);
	void createViewportState(vk::PipelineViewportStateCreateInfo& viewportState, vk::Viewport& viewport, vk::Rect2D& scissor);
	void createRasterizerState(vk::PipelineRasterizationStateCreateInfo& rasterizer);
	void createMultisampleState(vk::PipelineMultisampleStateCreateInfo& multisampling);
	void createColorBlendState(vk::PipelineColorBlendStateCreateInfo& colorBlending, vk::PipelineColorBlendAttachmentState& colorBlendAttachment);

	void createPipelineLayout();

	void createPipeline(
		const std::array<vk::PipelineShaderStageCreateInfo, 2>& shaderStages,
		const vk::PipelineVertexInputStateCreateInfo& vertexInputInfo,
		const vk::PipelineInputAssemblyStateCreateInfo& inputAssembly,
		const vk::PipelineViewportStateCreateInfo& viewportState,
		const vk::PipelineRasterizationStateCreateInfo& rasterizer,
		const vk::PipelineMultisampleStateCreateInfo& multisampling,
		const vk::PipelineColorBlendStateCreateInfo& colorBlending,
		VulkanSurface& surface);

	//TODO: This might be better in a helper class of some sort, maybe passed in as part of the context?
	vk::Format determineFormat(const SimpleRTTR::Type& type);
	vk::ShaderStageFlags determineStage(ShaderStage type);

public:
	VulkanPipeline(VulkanContext& context, const PipelineDefinition& pipelineDefinition);
	virtual ~VulkanPipeline();

	vk::Pipeline& getPipeline() { return _pipeline; }
	vk::PipelineLayout& getPipelineLayout() { return _pipelineLayout; }

};

} // namespace OpenXcom
