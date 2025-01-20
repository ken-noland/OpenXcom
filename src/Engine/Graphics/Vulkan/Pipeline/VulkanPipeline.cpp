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
#include "../VulkanDescriptorSet.h"
#include "../../../Resource/Pipeline/PipelineDefinition.h"


namespace OpenXcom
{

VulkanPipeline::VulkanPipeline(VulkanContext& context, const PipelineDefinition& pipelineDefinition)
	: _context(context)
{
	// let's start with the descriptor set layout
	_descriptorSetLayout = _context.getDescriptorSetFactory().createDescriptorSetLayout(pipelineDefinition);

	// now we can create the pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;

	_pipelineLayout = _context.getDevice().createPipelineLayout(pipelineLayoutInfo);

	// now we can create the pipeline
	uint32_t vertexShaderModule = pipelineDefinition.getVertexShader();
	uint32_t fragmentShaderModule = pipelineDefinition.getFragmentShader();

	// get the shader manager
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
