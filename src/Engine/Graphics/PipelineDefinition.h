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
#ifdef __linux__
#undef None // Xlib.h defines None, which conflicts with Vulkan
#endif

#include <simplerttr.h>
#include <assert.h>

namespace OpenXcom
{

class RenderTarget;
class Shader;

enum class ShaderStage : int
{
	Vertex,
	Fragment,
	Count
};

enum class PrimitiveTopology : int
{
	TriangleList,
	TriangleStrip,
	LineList,
	LineStrip,
	PointList,
	Count
};

struct BufferDefinition
{
	uint32_t binding;
	ShaderStage stage;
	SimpleRTTR::TypeReference type;
};

struct PushConstantDefinition
{
	ShaderStage stage;
	SimpleRTTR::TypeReference type;
};

struct TextureDefinition
{
	uint32_t binding;
	ShaderStage stage;
};

struct CombinedImageSamplerDefinition
{
	uint32_t binding;
	ShaderStage stage;
};

class ResourceLayoutDefinition
{
protected:
	SimpleRTTR::TypeReference _vertexType;
	SimpleRTTR::TypeReference _indexType;

	std::vector<BufferDefinition> _uniformBuffers;
	std::vector<BufferDefinition> _storageBuffers;
	std::vector<PushConstantDefinition> _pushConstants;
	std::vector<TextureDefinition> _textures;

	PrimitiveTopology _topology;

	std::vector<CombinedImageSamplerDefinition> _combinedImageSamplers;

public:
	ResourceLayoutDefinition()
		:
		  _vertexType(SimpleRTTR::types().get_type<void>().value()),
		  _indexType(SimpleRTTR::types().get_type<void>().value()),
		  _topology(PrimitiveTopology::TriangleList)
	{ }
	~ResourceLayoutDefinition() = default;

	void setVertexType(const SimpleRTTR::Type& type) { _vertexType = type; }
	SimpleRTTR::Type getVertexType() const { return _vertexType.type(); }

	void setIndexType(const SimpleRTTR::Type& type) { _indexType = type; }
	SimpleRTTR::Type getIndexType() const { return _indexType.type(); }

	void addUniformBuffer(const BufferDefinition& uniformBuffer) { _uniformBuffers.push_back(uniformBuffer); }
	const std::vector<BufferDefinition>& getUniformBuffers() const { return _uniformBuffers; }

	void addStorageBuffer(const BufferDefinition& storageBuffer) { _storageBuffers.push_back(storageBuffer); }
	const std::vector<BufferDefinition>& getStorageBuffers() const { return _storageBuffers; }

	void addPushConstant(const PushConstantDefinition& pushConstant) { _pushConstants.push_back(pushConstant); }
	const std::vector<PushConstantDefinition>& getPushConstants() const { return _pushConstants; }

	void addTexture(const TextureDefinition& texture) { _textures.push_back(texture); }
	const std::vector<TextureDefinition>& getTextures() const { return _textures; }

	void setTopology(PrimitiveTopology topology) { _topology = topology; }
	PrimitiveTopology getTopology() const { return _topology; }

	void addCombinedImageSampler(uint32_t binding, ShaderStage stage) { _combinedImageSamplers.push_back({binding, stage}); }
	const std::vector<CombinedImageSamplerDefinition>& getCombinedImageSamplers() const { return _combinedImageSamplers; }
};

class ResourceLayoutBuilder
{
protected:
	ResourceLayoutDefinition _resourceLayout;

public:
	ResourceLayoutBuilder() = default;
	~ResourceLayoutBuilder() = default;

	template <typename VertexType>
	ResourceLayoutBuilder& setVertexType();
	ResourceLayoutBuilder& setVertexType(const SimpleRTTR::Type& type)
	{
		_resourceLayout.setVertexType(type);
		return *this;
	}

	template <typename IndexType>
	ResourceLayoutBuilder& setIndexType();
	ResourceLayoutBuilder& setIndexType(const SimpleRTTR::Type& type)
	{
		_resourceLayout.setIndexType(type);
		return *this;
	}

	template <typename VertexType>
	ResourceLayoutBuilder& addUniformBuffer(ShaderStage stage, uint32_t binding);
	ResourceLayoutBuilder& addUniformBuffer(const SimpleRTTR::Type& type, ShaderStage stage, uint32_t binding)
	{
		_resourceLayout.addUniformBuffer({binding, stage, type});
		return *this;
	}

	template <typename VertexType>
	ResourceLayoutBuilder& addStorageBuffer(ShaderStage stage, uint32_t binding);
	ResourceLayoutBuilder& addStorageBuffer(const SimpleRTTR::Type& type, ShaderStage stage, uint32_t binding)
	{
		_resourceLayout.addStorageBuffer({binding, stage, type});
		return *this;
	}

	template <typename VertexType>
	ResourceLayoutBuilder& addPushConstant(ShaderStage stage);
	ResourceLayoutBuilder& addPushConstant(const SimpleRTTR::Type& type, ShaderStage stage)
	{
		_resourceLayout.addPushConstant({stage, type});
		return *this;
	}

	ResourceLayoutBuilder& addTexture(uint32_t binding, ShaderStage stage)
	{
		_resourceLayout.addTexture({binding, stage});
		return *this;
	}

	ResourceLayoutBuilder& setTopology(PrimitiveTopology topology)
	{
		_resourceLayout.setTopology(topology);
		return *this;
	}

	ResourceLayoutBuilder& addCombinedImageSampler(uint32_t binding, ShaderStage stage) // TODO: Specify the sampler parameters
	{
		_resourceLayout.addCombinedImageSampler(binding, stage);
		return *this;
	}

	ResourceLayoutDefinition build() { return _resourceLayout; };
};

template <typename VertexType>
ResourceLayoutBuilder& ResourceLayoutBuilder::setVertexType()
{
	assert(SimpleRTTR::types().has_type<VertexType>() && "IndexType not registered with SimpleRTTR");
	return setVertexType(SimpleRTTR::types().get_type<VertexType>().value());
}

template <typename IndexType>
ResourceLayoutBuilder& ResourceLayoutBuilder::setIndexType()
{
	assert(SimpleRTTR::types().has_type<IndexType>() && "IndexType not registered with SimpleRTTR");
	return setIndexType(SimpleRTTR::types().get_type<IndexType>().value());
}

template <typename UniformType>
ResourceLayoutBuilder& ResourceLayoutBuilder::addUniformBuffer(ShaderStage stage, uint32_t binding)
{
	assert(SimpleRTTR::types().has_type<UniformType>() && "UniformType not registered with SimpleRTTR");
	return addUniformBuffer(SimpleRTTR::types().get_type<UniformType>().value(), stage, binding);
}

template <typename StorageType>
ResourceLayoutBuilder& ResourceLayoutBuilder::addStorageBuffer(ShaderStage stage, uint32_t binding)
{
	assert(SimpleRTTR::types().has_type<StorageType>() && "UniformType not registered with SimpleRTTR");
	return addStorageBuffer(SimpleRTTR::types().get_type<StorageType>().value(), stage, binding);
}

template <typename PushConstantType>
ResourceLayoutBuilder& ResourceLayoutBuilder::addPushConstant(ShaderStage stage)
{
	assert(SimpleRTTR::types().has_type<PushConstantType>() && "PushConstantType not registered with SimpleRTTR");
	return addPushConstant(SimpleRTTR::types().get_type<PushConstantType>().value(), stage);
}

class PipelineDefinition
{
protected:
	std::optional<std::reference_wrapper<const Shader>> _vertexShader;
	std::optional<std::reference_wrapper<const Shader>> _fragmentShader;

	std::optional<std::reference_wrapper<const RenderTarget>> _renderTarget;

	ResourceLayoutDefinition _resourceLayout;

public:
	PipelineDefinition() = default;
	~PipelineDefinition() = default;

	void setVertexShader(const Shader& shader) { _vertexShader = shader; }
	const Shader& getVertexShader() const { return _vertexShader.value().get(); }

	void setFragmentShader(const Shader& shader) { _fragmentShader = shader; }
	const Shader& getFragmentShader() const { return _fragmentShader.value().get(); }

	void setResourceLayout(const ResourceLayoutDefinition& resourceLayout) { _resourceLayout = resourceLayout; }
	const ResourceLayoutDefinition& getResourceLayout() const { return _resourceLayout; }

	void setRenderTarget(const RenderTarget& renderTarget) { _renderTarget = renderTarget; }
	bool hasRenderTarget() const { return _renderTarget.has_value(); }
	const RenderTarget& getSurface() const
	{
		assert(_renderTarget.has_value() && "You must set the render target before calling this function.");
		return _renderTarget.value().get();
	}
};

class PipelineBuilder
{
	PipelineDefinition _pipelineDefinition;

public:
	PipelineBuilder() = default;
	~PipelineBuilder() = default;

	PipelineBuilder& setVertexShader(const Shader& shader)
	{
		_pipelineDefinition.setVertexShader(shader);
		return *this;
	}

	PipelineBuilder& setFragmentShader(const Shader& shader)
	{
		_pipelineDefinition.setFragmentShader(shader);
		return *this;
	}

	PipelineBuilder& setResourceLayout(const ResourceLayoutDefinition& resourceLayoutDefinition)
	{
		_pipelineDefinition.setResourceLayout(resourceLayoutDefinition);
		return *this;
	}

	PipelineBuilder& setRenderTarget(const RenderTarget& surface)
	{
		_pipelineDefinition.setRenderTarget(surface);
		return *this;
	}

	PipelineDefinition build() { return _pipelineDefinition; }
};

} // namespace OpenXcom
