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
#include <vulkan/vulkan.hpp>
#include <optional>

namespace OpenXcom
{

class VulkanContext;
class VulkanPipeline;
class VulkanDescriptorSet;




class VulkanPipelineBinding : public PipelineBinding
{
protected:
	VulkanContext& _context;
	VulkanPipeline& _pipeline;

	std::unique_ptr<VulkanDescriptorSet> _descriptorSet;

	std::optional<std::reference_wrapper<DeviceBuffer>> _vertexBuffer;
	std::optional<std::reference_wrapper<DeviceBuffer>> _indexBuffer;

	void create();
	void destroy();
	void recreate();

public:
	VulkanPipelineBinding(VulkanContext& context, VulkanPipeline& pipeline);
	virtual ~VulkanPipelineBinding();

	virtual void setVertexBuffer(DeviceBuffer& buffer) override;
	virtual void setIndexBuffer(DeviceBuffer& buffer) override;

	virtual void setPushConstant(SimpleRTTR::Type& type, ShaderStage stage, const void* data, std::size_t size) override;

	virtual void setTexture(ShaderStage stage, uint32_t binding, Image& image);

};

} // namespace OpenXcom
