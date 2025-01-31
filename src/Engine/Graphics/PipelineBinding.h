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
#include <cstdint>
#include <simplerttr.h>

namespace OpenXcom
{

class DeviceBuffer;
class DeviceImage;
class GraphicsCommand;
enum class ShaderStage;

class PipelineBinding
{
public:
	PipelineBinding() = default;
	virtual ~PipelineBinding() = default;

	virtual void setVertexBuffer(DeviceBuffer& buffer) = 0;
	virtual void setIndexBuffer(DeviceBuffer& buffer) = 0;
	virtual void setUniformBuffer(ShaderStage stage, uint32_t binding, const DeviceBuffer& buffer) = 0;

	template <typename ConstantType>
	void setPushConstant(ShaderStage stage, const ConstantType& data);
	virtual void setPushConstant(const SimpleRTTR::Type& type, ShaderStage stage, const void* data, std::size_t size) = 0;

	virtual void setTexture(ShaderStage stage, uint32_t binding, const DeviceImage& image) = 0;

	virtual void commit(GraphicsCommand& command) = 0;
};


template <typename ConstantType>
void PipelineBinding::setPushConstant(ShaderStage stage, const ConstantType& data)
{
	setPushConstant(SimpleRTTR::types().get_type<ConstantType>().value(), stage, &data, sizeof(ConstantType));
}

} // namespace OpenXcom
