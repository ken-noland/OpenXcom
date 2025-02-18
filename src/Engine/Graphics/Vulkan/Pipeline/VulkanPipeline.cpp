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
#include "VulkanPipeline.h"
#include "VulkanPipelineBinding.h"
#include "../VulkanContext.h"
#include "../VulkanDescriptorSet.h"
#include "../Image/VulkanRenderTarget.h"
#include "../Shader/VulkanShader.h"
#include "../Surface/VulkanWindowedSurface.h"
#include "../../PipelineDefinition.h"
#include "../../../Resource/ResourceSystem.h"
#include "../../../Engine.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace OpenXcom
{

VulkanPipeline::VulkanPipeline(VulkanContext& context, const PipelineDefinition& pipelineDefinition)
	: _context(context), _pipelineDefinition(pipelineDefinition)
{
	assert(pipelineDefinition.hasRenderTarget() && "You must specify the render target or surface in the pipeline definition");

	const RenderTarget& renderTarget = pipelineDefinition.getSurface();

	createDescriptorSetLayout();

	auto shaderStages = createShaderStages();

	vk::VertexInputBindingDescription bindingDescription{};
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

	vk::Viewport viewport;
	vk::Rect2D scissor;

	vk::PipelineColorBlendAttachmentState colorBlendAttachment;
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
	vk::PipelineViewportStateCreateInfo viewportState{};
	vk::PipelineRasterizationStateCreateInfo rasterizer{};
	vk::PipelineMultisampleStateCreateInfo multisampling{};
	vk::PipelineColorBlendStateCreateInfo colorBlending{};

	createVertexInputInfo(bindingDescription, attributeDescriptions, vertexInputInfo);
	createInputAssemblyState(inputAssembly);
	createViewportState(viewportState, viewport, scissor, renderTarget);
	createRasterizerState(rasterizer);
	createMultisampleState(multisampling);
	createColorBlendState(colorBlending, colorBlendAttachment);

	//temp
	vk::PipelineRasterizationLineStateCreateInfoEXT lineRasterizationState{};
	lineRasterizationState.lineRasterizationMode = vk::LineRasterizationModeEXT::eBresenham;
	rasterizer.pNext = &lineRasterizationState;

	createPipelineLayout();

	createPipeline(shaderStages, vertexInputInfo, inputAssembly, viewportState, rasterizer, multisampling, colorBlending, renderTarget);
}

void VulkanPipeline::createDescriptorSetLayout()
{
	_descriptorSetLayout = _context.getDescriptorSetFactory().createDescriptorSetLayout(_pipelineDefinition);
}

std::array<vk::PipelineShaderStageCreateInfo, 2> VulkanPipeline::createShaderStages()
{
	const VulkanShader& vertexShaderModule = static_cast<const VulkanShader&>(_pipelineDefinition.getVertexShader());
	const VulkanShader& fragmentShaderModule = static_cast<const VulkanShader&>(_pipelineDefinition.getFragmentShader());

	const vk::ShaderModule& vertexShader = vertexShaderModule.module();
	const vk::ShaderModule& fragmentShader = fragmentShaderModule.module();

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
	vertShaderStageInfo.module = vertexShader;
	vertShaderStageInfo.pName = "main";

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
	fragShaderStageInfo.module = fragmentShader;
	fragShaderStageInfo.pName = "main";

	return {vertShaderStageInfo, fragShaderStageInfo};
}

void VulkanPipeline::createVertexInputInfo(
	vk::VertexInputBindingDescription& bindingDescription,
	std::vector<vk::VertexInputAttributeDescription>& attributeDescriptions,
	vk::PipelineVertexInputStateCreateInfo& vertexInputInfo)
{
	SimpleRTTR::Type vertexType = _pipelineDefinition.getResourceLayout().getVertexType();

	bindingDescription.binding = 0;
	bindingDescription.stride = static_cast<uint32_t>(vertexType.size());
	bindingDescription.inputRate = vk::VertexInputRate::eVertex;

	attributeDescriptions.resize(vertexType.properties().size());
	int location = 0;
	for (const auto& property : vertexType.properties())
	{
		vk::VertexInputAttributeDescription attributeDescription{};
		attributeDescription.binding = 0;
		attributeDescription.location = location;
		attributeDescription.format = determineFormat(property.type());
		attributeDescription.offset = static_cast<uint32_t>(property.offset());
		attributeDescriptions[location++] = attributeDescription;
	}

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
}

void VulkanPipeline::createInputAssemblyState(vk::PipelineInputAssemblyStateCreateInfo& inputAssembly)
{
	PrimitiveTopology topology = _pipelineDefinition.getResourceLayout().getTopology();

	switch(topology)
	{
	case PrimitiveTopology::TriangleList:
		inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
		break;
	case PrimitiveTopology::TriangleStrip:
		inputAssembly.topology = vk::PrimitiveTopology::eTriangleStrip;
		break;
	case PrimitiveTopology::LineList:
		inputAssembly.topology = vk::PrimitiveTopology::eLineList;
		break;
	case PrimitiveTopology::LineStrip:
		inputAssembly.topology = vk::PrimitiveTopology::eLineStrip;
		break;
	case PrimitiveTopology::PointList:
		inputAssembly.topology = vk::PrimitiveTopology::ePointList;
		break;
	default:
		throw std::runtime_error("Unsupported primitive topology");
	}

	inputAssembly.primitiveRestartEnable = VK_FALSE;
}

void VulkanPipeline::createViewportState(vk::PipelineViewportStateCreateInfo& viewportState, vk::Viewport& viewport, vk::Rect2D& scissor, const RenderTarget& surface)
{
	glm::ivec2 surfaceExtent = surface.getExtent();

	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)surfaceExtent.x;
	viewport.height = (float)surfaceExtent.y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = vk::Extent2D(surfaceExtent.x, surfaceExtent.y);

	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;
}

void VulkanPipeline::createRasterizerState(vk::PipelineRasterizationStateCreateInfo& rasterizer)
{
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;

	if(_pipelineDefinition.getResourceLayout().getTopology() == PrimitiveTopology::LineList ||
		_pipelineDefinition.getResourceLayout().getTopology() == PrimitiveTopology::LineStrip)
	{
		rasterizer.polygonMode = vk::PolygonMode::eLine;
	}
	else if (_pipelineDefinition.getResourceLayout().getTopology() == PrimitiveTopology::PointList)
	{
		rasterizer.polygonMode = vk::PolygonMode::ePoint;
	}
	else
	{
		rasterizer.polygonMode = vk::PolygonMode::eFill;
	}

	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = vk::CullModeFlagBits::eNone;
	rasterizer.frontFace = vk::FrontFace::eClockwise;
	rasterizer.depthBiasEnable = VK_FALSE;
}

void VulkanPipeline::createMultisampleState(vk::PipelineMultisampleStateCreateInfo& multisampling)
{
	uint32_t sampleCount = _pipelineDefinition.getSurface().getMultisampleCount();
	if (sampleCount > 1)
	{
		vk::SampleCountFlagBits sampleCountFlagBits = static_cast<vk::SampleCountFlagBits>(sampleCount);
		multisampling.sampleShadingEnable = VK_TRUE;
		multisampling.rasterizationSamples = sampleCountFlagBits;
	}
	else
	{
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
	}
}

void VulkanPipeline::createColorBlendState(vk::PipelineColorBlendStateCreateInfo& colorBlending, vk::PipelineColorBlendAttachmentState& colorBlendAttachment)
{
	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	colorBlendAttachment.blendEnable = VK_FALSE;

	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = vk::LogicOp::eCopy;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
}

void VulkanPipeline::createPipelineLayout()
{
	const std::vector<PushConstantDefinition>& pushConstants = _pipelineDefinition.getResourceLayout().getPushConstants();

	std::vector<vk::PushConstantRange> pushConstantRanges;
	for (const auto& pushConstant : pushConstants)
	{
		vk::PushConstantRange range{};
		range.stageFlags = determineStage(pushConstant.stage);
		range.offset = 0; // Assuming offset is 0 for simplicity
		range.size = static_cast<uint32_t>(pushConstant.type.type().size());
		pushConstantRanges.push_back(range);
	}

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
	pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

	_pipelineLayout = _context.getDevice().createPipelineLayout(pipelineLayoutInfo);
}

void VulkanPipeline::createPipeline(
	const std::array<vk::PipelineShaderStageCreateInfo, 2>& shaderStages,
	const vk::PipelineVertexInputStateCreateInfo& vertexInputInfo,
	const vk::PipelineInputAssemblyStateCreateInfo& inputAssembly,
	const vk::PipelineViewportStateCreateInfo& viewportState,
	const vk::PipelineRasterizationStateCreateInfo& rasterizer,
	const vk::PipelineMultisampleStateCreateInfo& multisampling,
	const vk::PipelineColorBlendStateCreateInfo& colorBlending,
	const RenderTarget& surface)
{
	vk::RenderPass renderPass;

	switch(surface.getType())
	{
	case ImageType::Surface:
	{
		// check if the surface is windowed or headless
		const VulkanSurface& vulkanSurface = static_cast<const VulkanSurface&>(surface);
		if(vulkanSurface.getVulkanSurfaceType() == VulkanSurfaceType::Windowed)
		{
			renderPass = static_cast<const VulkanWindowedSurface&>(surface).getRenderPass();
		}
		else
		{
			assert(false && "Attempting to create renderpass on headless surface is not allowed");
		}
		break;
	}
	case ImageType::RenderTarget:
	{
		renderPass = static_cast<const VulkanRenderTarget&>(surface).getRenderPass();
		break;
	}
	case ImageType::Texture: {
		assert(false && "Attempting to create renderpass on texture is not allowed");
		break;
	}
	}

	vk::GraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = _pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;

	// Declare the dynamic states
	std::vector<vk::DynamicState> dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor};

	vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};

	if (surface.getUseDynamicStates())
	{
		dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

		pipelineInfo.pDynamicState = &dynamicStateCreateInfo;
	}
	else
	{
		pipelineInfo.pDynamicState = nullptr;
	}

	// Create the pipeline
	vk::ResultValue<vk::Pipeline> result = _context.getDevice().createGraphicsPipeline(nullptr, pipelineInfo);
	if (result.result == vk::Result::eSuccess)
	{
		_pipeline = result.value;
	}
	else
	{
		throw std::runtime_error("Failed to create graphics pipeline");
	}
}

vk::Format VulkanPipeline::determineFormat(const SimpleRTTR::Type& type)
{
	//TODO: This needs to be moved to a map lookup. It could be part of _context so that we only have to do the type looksup once
	if (type == SimpleRTTR::types().get_type<glm::vec2>())
	{
		return vk::Format::eR32G32Sfloat;
	}
	else if (type == SimpleRTTR::types().get_type<glm::ivec2>())
	{
		return vk::Format::eR32G32Sint;
	}
	else if (type == SimpleRTTR::types().get_type<glm::vec3>())
	{
		return vk::Format::eR32G32B32Sfloat;
	}
	else if (type == SimpleRTTR::types().get_type<glm::vec4>())
	{
		return vk::Format::eR32G32B32A32Sfloat;
	}
	else if (type == SimpleRTTR::types().get_type<unsigned int>())
	{
		return vk::Format::eR32Uint;
	}
	else if (type == SimpleRTTR::types().get_type<int>())
	{
		return vk::Format::eR32Sint;
	}
	else
	{
		throw std::runtime_error("Unsupported vertex type");
	}
}

vk::ShaderStageFlags VulkanPipeline::determineStage(ShaderStage type)
{
	switch (type)
	{
	case ShaderStage::Vertex:
		return vk::ShaderStageFlagBits::eVertex;
	case ShaderStage::Fragment:
		return vk::ShaderStageFlagBits::eFragment;
	default:
		throw std::runtime_error("Unsupported shader stage");
	}
}


VulkanPipeline::~VulkanPipeline()
{
	_context.getDevice().destroyPipeline(_pipeline);
	_context.getDevice().destroyPipelineLayout(_pipelineLayout);
	_context.getDescriptorSetFactory().destroyDescriptorSetLayout(_descriptorSetLayout);
}

std::unique_ptr<PipelineBinding> VulkanPipeline::createBinding()
{
	return std::make_unique<VulkanPipelineBinding>(_context, *this);
}

} // namespace OpenXcom
