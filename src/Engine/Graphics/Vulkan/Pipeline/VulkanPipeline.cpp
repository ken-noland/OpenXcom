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
#include "../VulkanContext.h"
#include "../VulkanSurface.h"
#include "../VulkanDescriptorSet.h"
#include "../Shader/VulkanShaderManager.h"
#include "../../../Resource/Pipeline/PipelineDefinition.h"
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
	VulkanSurface& surface = static_cast<VulkanSurface&>(pipelineDefinition.getSurface());

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
	createViewportState(viewportState, viewport, scissor);
	createRasterizerState(rasterizer);
	createMultisampleState(multisampling);
	createColorBlendState(colorBlending, colorBlendAttachment);

	createPipelineLayout();

	createPipeline(shaderStages, vertexInputInfo, inputAssembly, viewportState, rasterizer, multisampling, colorBlending, surface);
}

void VulkanPipeline::createDescriptorSetLayout()
{
	_descriptorSetLayout = _context.getDescriptorSetFactory().createDescriptorSetLayout(_pipelineDefinition);
}

std::array<vk::PipelineShaderStageCreateInfo, 2> VulkanPipeline::createShaderStages()
{
	VulkanShaderManager& shaderManager = static_cast<VulkanShaderManager&>(getEngine().getResourceSystem().getShaderManager());

	uint32_t vertexShaderModule = _pipelineDefinition.getVertexShader();
	uint32_t fragmentShaderModule = _pipelineDefinition.getFragmentShader();

	const vk::ShaderModule& vertexShader = shaderManager.get<VulkanShader>(vertexShaderModule).module();
	const vk::ShaderModule& fragmentShader = shaderManager.get<VulkanShader>(fragmentShaderModule).module();

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
	inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
}

void VulkanPipeline::createViewportState(vk::PipelineViewportStateCreateInfo& viewportState, vk::Viewport& viewport, vk::Rect2D& scissor)
{
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = 1.0f;
	viewport.height = 1.0f;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor.offset = vk::Offset2D{0, 0};
	scissor.extent = vk::Extent2D{1, 1};

	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;
}

void VulkanPipeline::createRasterizerState(vk::PipelineRasterizationStateCreateInfo& rasterizer)
{
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = vk::PolygonMode::eFill;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = vk::CullModeFlagBits::eBack;
	rasterizer.frontFace = vk::FrontFace::eClockwise;
	rasterizer.depthBiasEnable = VK_FALSE;
}

void VulkanPipeline::createMultisampleState(vk::PipelineMultisampleStateCreateInfo& multisampling)
{
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
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
	VulkanSurface& surface)
{

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
	pipelineInfo.renderPass = surface.getRenderPass();
	pipelineInfo.subpass = 0;

	_pipeline = _context.getDevice().createGraphicsPipeline(nullptr, pipelineInfo).value;
}

vk::Format VulkanPipeline::determineFormat(const SimpleRTTR::Type& type)
{
	if (type == SimpleRTTR::types().get_type<glm::vec2>())
	{
		return vk::Format::eR32G32Sfloat;
	}
	else if (type == SimpleRTTR::types().get_type<glm::vec3>())
	{
		return vk::Format::eR32G32B32Sfloat;
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
	_context.getDescriptorSetFactory().destroyDescriptorSetLayout(_descriptorSetLayout);
	_context.getDevice().destroyPipelineLayout(_pipelineLayout);
}


//
///*
//
//	const vk::ShaderModule& vertexShaderModule = shaderManager.get<VulkanShader>(_vertexShader).module();
//	const vk::ShaderModule& fragmentShaderModule = shaderManager.get<VulkanShader>(_fragmentShader).module();
//
//	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
//	vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
//	vertShaderStageInfo.module = vertexShaderModule;
//	vertShaderStageInfo.pName = "main"; // Entry point in the shader
//
//	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
//	fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
//	fragShaderStageInfo.module = fragmentShaderModule;
//	fragShaderStageInfo.pName = "main";
//
//	vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
//
//	// Vertex definitions
//	SimpleRTTR::Type vertexType = SimpleRTTR::types().get_type<Vertex>().value();
//
//	vk::VertexInputBindingDescription bindingDescription{};
//	bindingDescription.binding = 0;
//	bindingDescription.stride = sizeof(Vertex);
//	bindingDescription.inputRate = vk::VertexInputRate::eVertex;
//
//	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
//	attributeDescriptions.resize(vertexType.properties().size());
//
//	int location = 0;
//	for (const SimpleRTTR::Property& property : vertexType.properties())
//	{
//		const SimpleRTTR::Type& propertyType = property.type();
//		const vk::Format& format = propertyType.meta().get("format").value().get_as<vk::Format>();
//
//		vk::VertexInputAttributeDescription attributeDescription{};
//		attributeDescription.binding = 0;
//		attributeDescription.location = location;
//		attributeDescription.format = format;
//		attributeDescription.offset = (uint32_t)property.offset();
//		attributeDescriptions[location] = attributeDescription;
//		location++;
//	}
//
//	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
//	vertexInputInfo.vertexBindingDescriptionCount = 1;
//	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
//	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
//	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
//
//	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
//	inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
//	inputAssembly.primitiveRestartEnable = VK_FALSE;
//
//	vk::Viewport viewport{};
//	viewport.x = 0.0f;
//	viewport.y = 0.0f;
//	viewport.width = static_cast<float>(_swapChainExtent.width);
//	viewport.height = static_cast<float>(_swapChainExtent.height);
//	viewport.minDepth = 0.0f;
//	viewport.maxDepth = 1.0f;
//
//	vk::Rect2D scissor{};
//	scissor.offset = vk::Offset2D{0, 0};
//	scissor.extent = _swapChainExtent;
//
//	vk::PipelineViewportStateCreateInfo viewportState{};
//	viewportState.viewportCount = 1;
//	viewportState.pViewports = &viewport;
//	viewportState.scissorCount = 1;
//	viewportState.pScissors = &scissor;
//
//	vk::PipelineRasterizationStateCreateInfo rasterizer{};
//	rasterizer.depthClampEnable = VK_FALSE;
//	rasterizer.rasterizerDiscardEnable = VK_FALSE;
//	rasterizer.polygonMode = vk::PolygonMode::eFill; // Fill polygons
//	rasterizer.lineWidth = 1.0f;
//	rasterizer.cullMode = vk::CullModeFlagBits::eBack;
//	rasterizer.frontFace = vk::FrontFace::eClockwise;
//	rasterizer.depthBiasEnable = VK_FALSE;
//
//	vk::PipelineMultisampleStateCreateInfo multisampling{};
//	multisampling.sampleShadingEnable = VK_FALSE;
//	multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
//
//	vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
//	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR |
//										  vk::ColorComponentFlagBits::eG |
//										  vk::ColorComponentFlagBits::eB |
//										  vk::ColorComponentFlagBits::eA;
//	colorBlendAttachment.blendEnable = VK_FALSE;
//
//	vk::PipelineColorBlendStateCreateInfo colorBlending{};
//	colorBlending.logicOpEnable = VK_FALSE;
//	colorBlending.logicOp = vk::LogicOp::eCopy;
//	colorBlending.attachmentCount = 1;
//	colorBlending.pAttachments = &colorBlendAttachment;
//
//	vk::PushConstantRange pushConstantRange{};
//	pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex;
//	pushConstantRange.offset = 0;
//	pushConstantRange.size = sizeof(glm::mat4);
//
//	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
//	pipelineLayoutInfo.setLayoutCount = 1;
//	pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;
//
//	pipelineLayoutInfo.pushConstantRangeCount = 1;
//	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
//
//	_pipelineLayout = _device.createPipelineLayout(pipelineLayoutInfo);
//
//	vk::GraphicsPipelineCreateInfo pipelineInfo{};
//	pipelineInfo.stageCount = 2;
//	pipelineInfo.pStages = shaderStages;
//	pipelineInfo.pVertexInputState = &vertexInputInfo;
//	pipelineInfo.pInputAssemblyState = &inputAssembly;
//	pipelineInfo.pViewportState = &viewportState;
//	pipelineInfo.pRasterizationState = &rasterizer;
//	pipelineInfo.pMultisampleState = &multisampling;
//	pipelineInfo.pColorBlendState = &colorBlending;
//	pipelineInfo.layout = _pipelineLayout;
//	pipelineInfo.renderPass = _renderPass;
//	pipelineInfo.subpass = 0;
//
//	_pipeline = _device.createGraphicsPipeline(nullptr, pipelineInfo).value;
//
//
//
//*/
//
//VulkanPipelineManager::VulkanPipelineManager()
//{
//}
//
//VulkanPipelineManager::~VulkanPipelineManager()
//{
//}
//
//vk::Pipeline VulkanPipelineManager::createPipeline(const PipelineBuilder& builder)
//{
//
//	const vk::ShaderModule& vertexShaderModule = shaderManager.get<VulkanShader>(_vertexShader).module();
//	const vk::ShaderModule& fragmentShaderModule = shaderManager.get<VulkanShader>(_fragmentShader).module();
//
//	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
//	vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
//	vertShaderStageInfo.module = vertexShaderModule;
//	vertShaderStageInfo.pName = "main"; // Entry point in the shader
//
//	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
//	fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
//	fragShaderStageInfo.module = fragmentShaderModule;
//	fragShaderStageInfo.pName = "main";
//
//	vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
//
//	// Vertex definitions
//	SimpleRTTR::Type vertexType = SimpleRTTR::types().get_type<Vertex>().value();
//
//	vk::VertexInputBindingDescription bindingDescription{};
//	bindingDescription.binding = 0;
//	bindingDescription.stride = sizeof(Vertex);
//	bindingDescription.inputRate = vk::VertexInputRate::eVertex;
//
//	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
//	attributeDescriptions.resize(vertexType.properties().size());
//
//	int location = 0;
//	for (const SimpleRTTR::Property& property : vertexType.properties())
//	{
//		const SimpleRTTR::Type& propertyType = property.type();
//		const vk::Format& format = propertyType.meta().get("format").value().get_as<vk::Format>();
//
//		vk::VertexInputAttributeDescription attributeDescription{};
//		attributeDescription.binding = 0;
//		attributeDescription.location = location;
//		attributeDescription.format = format;
//		attributeDescription.offset = (uint32_t)property.offset();
//		attributeDescriptions[location] = attributeDescription;
//		location++;
//	}
//
//	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
//	vertexInputInfo.vertexBindingDescriptionCount = 1;
//	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
//	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
//	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
//
//	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
//	inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
//	inputAssembly.primitiveRestartEnable = VK_FALSE;
//
//	vk::Viewport viewport{};
//	viewport.x = 0.0f;
//	viewport.y = 0.0f;
//	viewport.width = static_cast<float>(_swapChainExtent.width);
//	viewport.height = static_cast<float>(_swapChainExtent.height);
//	viewport.minDepth = 0.0f;
//	viewport.maxDepth = 1.0f;
//
//	vk::Rect2D scissor{};
//	scissor.offset = vk::Offset2D{0, 0};
//	scissor.extent = _swapChainExtent;
//
//	vk::PipelineViewportStateCreateInfo viewportState{};
//	viewportState.viewportCount = 1;
//	viewportState.pViewports = &viewport;
//	viewportState.scissorCount = 1;
//	viewportState.pScissors = &scissor;
//
//	vk::PipelineRasterizationStateCreateInfo rasterizer{};
//	rasterizer.depthClampEnable = VK_FALSE;
//	rasterizer.rasterizerDiscardEnable = VK_FALSE;
//	rasterizer.polygonMode = vk::PolygonMode::eFill; // Fill polygons
//	rasterizer.lineWidth = 1.0f;
//	rasterizer.cullMode = vk::CullModeFlagBits::eBack;
//	rasterizer.frontFace = vk::FrontFace::eClockwise;
//	rasterizer.depthBiasEnable = VK_FALSE;
//
//	vk::PipelineMultisampleStateCreateInfo multisampling{};
//	multisampling.sampleShadingEnable = VK_FALSE;
//	multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
//
//	vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
//	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR |
//										  vk::ColorComponentFlagBits::eG |
//										  vk::ColorComponentFlagBits::eB |
//										  vk::ColorComponentFlagBits::eA;
//	colorBlendAttachment.blendEnable = VK_FALSE;
//
//	vk::PipelineColorBlendStateCreateInfo colorBlending{};
//	colorBlending.logicOpEnable = VK_FALSE;
//	colorBlending.logicOp = vk::LogicOp::eCopy;
//	colorBlending.attachmentCount = 1;
//	colorBlending.pAttachments = &colorBlendAttachment;
//
//	vk::PushConstantRange pushConstantRange{};
//	pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex;
//	pushConstantRange.offset = 0;
//	pushConstantRange.size = sizeof(glm::mat4);
//
//	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
//	pipelineLayoutInfo.setLayoutCount = 1;
//	pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;
//
//	pipelineLayoutInfo.pushConstantRangeCount = 1;
//	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
//
//	_pipelineLayout = _device.createPipelineLayout(pipelineLayoutInfo);
//
//	vk::GraphicsPipelineCreateInfo pipelineInfo{};
//	pipelineInfo.stageCount = 2;
//	pipelineInfo.pStages = shaderStages;
//	pipelineInfo.pVertexInputState = &vertexInputInfo;
//	pipelineInfo.pInputAssemblyState = &inputAssembly;
//	pipelineInfo.pViewportState = &viewportState;
//	pipelineInfo.pRasterizationState = &rasterizer;
//	pipelineInfo.pMultisampleState = &multisampling;
//	pipelineInfo.pColorBlendState = &colorBlending;
//	pipelineInfo.layout = _pipelineLayout;
//	pipelineInfo.renderPass = _renderPass;
//	pipelineInfo.subpass = 0;
//
//	return _device.createGraphicsPipeline(nullptr, pipelineInfo).value;
//}
//

} // namespace OpenXcom
