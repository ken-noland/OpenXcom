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

#include "../../PipelineBinding.h"
#include "../../PipelineDefinition.h"
#include <vulkan/vulkan.hpp>
#include <optional>

namespace OpenXcom
{

class VulkanContext;
class VulkanPipeline;
class VulkanDescriptorSet;
class VulkanDeviceBuffer;

// holder class for push constant data
class VulkanPushConstant
{
public:
	ShaderStage _stage;
	std::vector<uint8_t> _data;

public:
	VulkanPushConstant(ShaderStage stage, std::size_t size);
	~VulkanPushConstant();

	void* data() { return _data.data(); }
	std::size_t size() { return _data.size(); }

	void copyTo(const void* data, std::size_t size);
};

class VulkanPipelineBinding : public PipelineBinding
{
protected:
	VulkanContext& _context;
	VulkanPipeline& _pipeline;

	std::unique_ptr<VulkanDescriptorSet> _descriptorSet;

	std::array<std::vector<std::optional<std::reference_wrapper<VulkanDeviceBuffer>>>, (std::size_t)ShaderStage::Count> _uniformBuffers;
	std::array<std::unique_ptr<VulkanPushConstant>, (size_t)ShaderStage::Count> _pushConstants;

	std::optional<std::reference_wrapper<VulkanDeviceBuffer>> _vertexBuffer;
	std::optional<std::reference_wrapper<VulkanDeviceBuffer>> _indexBuffer;

	void create();
	void destroy();

	vk::ShaderStageFlagBits getShaderStage(ShaderStage stage);

public:
	VulkanPipelineBinding(VulkanContext& context, VulkanPipeline& pipeline);
	virtual ~VulkanPipelineBinding();

	virtual void setVertexBuffer(DeviceBuffer& buffer) override;
	virtual void setIndexBuffer(DeviceBuffer& buffer) override;
	virtual void setUniformBuffer(ShaderStage stage, uint32_t binding, DeviceBuffer& buffer) override;

	virtual void setPushConstant(const SimpleRTTR::Type& type, ShaderStage stage, const void* data, std::size_t size) override;

	virtual void setTexture(ShaderStage stage, uint32_t binding, const DeviceImage& image) override;

	virtual void commit(GraphicsCommand& command) override;
};

} // namespace OpenXcom
