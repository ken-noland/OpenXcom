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
#include "../GraphicsSystem.h"
#include "../GraphicsSurface.h"
#include "../GraphicsCommand.h"
#include "../ShaderManager.h"
#include "../Pipeline.h"
#include "../PipelineManager.h"
#include "../PipelineBinding.h"
#include "../Buffer/BufferManager.h"
#include "../Image/ImageManager.h"

// Null Graphics System
//
// This is a null implementation of the GraphicsSystem interface. Null graphics
// system should not be confused with headless mode, as running in headless mode
// still allows you to render to the game surface to capture the output. Null
// graphics system is a complete no-op, and is used for testing and debugging
// purposes.

namespace OpenXcom
{

class NullGraphicsCommand : public GraphicsCommand
{
public:
	NullGraphicsCommand() : GraphicsCommand() {};
	virtual ~NullGraphicsCommand() = default;

	virtual void beginRenderPass(RenderTarget& surface) override {}
	virtual void endRenderPass() override {}
};

class NullDeviceBuffer;
class NullHostBuffer : public HostBuffer
{
public:
	NullHostBuffer(BufferUsage usage, std::size_t elementSize) : HostBuffer(usage, elementSize) {};
	virtual ~NullHostBuffer() = default;

	virtual void resize(std::size_t count) override {}
	virtual void reserve(std::size_t count) override {}
	virtual void clear() override {}
	virtual void copy(DeviceBuffer& buffer) override {}
	virtual void copy(const void* data, std::size_t size) override {}
		
	virtual void* map() override { return nullptr; }
	virtual void unmap() override {}
};

class NullDeviceBuffer : public DeviceBuffer
{
public:
	NullDeviceBuffer(BufferUsage usage, std::size_t elementSize) : DeviceBuffer(usage, elementSize) {};
	virtual ~NullDeviceBuffer() = default;

	virtual void resize(std::size_t count) override {}
	virtual void reserve(std::size_t count) override {}
	virtual void clear() override {}
	virtual void copy(const HostBuffer& buffer) override {}
};

class NullHostImage : public HostImage
{
	void* image;

public:
	NullHostImage(glm::ivec2 size, ImageFormat format) : HostImage(ImageType::Texture)
	{
		image = malloc(size.x * size.y * bytesPerPixel(format));
	}
	virtual ~NullHostImage()
	{
		free(image);
	}

	virtual ImageFormat getFormat() const override { return ImageFormat::R8G8B8A8; }
	virtual glm::ivec2 getExtent() const override { return {-1, -1}; }
	virtual uint32_t getWidth() const override { return std::numeric_limits<uint32_t>::max(); }
	virtual uint32_t getHeight() const override { return std::numeric_limits<uint32_t>::max(); }

	ImageType getType() const { return _type; }

	virtual void* map() override { return image; }
	virtual void unmap() override {}
};

class NullDeviceImage : public DeviceImage
{
protected:
	NullDeviceBuffer _deviceBuffer;

public:
	NullDeviceImage(glm::ivec2 size, ImageFormat format) : DeviceImage(ImageType::Texture), _deviceBuffer(BufferUsage::Uniform, 1) {};
	virtual ~NullDeviceImage() = default;

	virtual ImageFormat getFormat() const override { return ImageFormat::R8G8B8A8; }
	virtual glm::ivec2 getExtent() const override { return {-1, -1}; }
	virtual uint32_t getWidth() const override { return std::numeric_limits<uint32_t>::max(); }
	virtual uint32_t getHeight() const override { return std::numeric_limits<uint32_t>::max(); }

	virtual void copyFrom(HostImage& hostImage) override {}
	virtual void copyTo(HostImage& hostImage) override {}

	virtual const DeviceBuffer& getDeviceImageData() const override { return _deviceBuffer; }
};

class NullGraphicsSurface : public GraphicsSurface
{
protected:
	NullDeviceBuffer _deviceBuffer;
	NullGraphicsCommand _command;

public:
	NullGraphicsSurface()
		: GraphicsSurface(), _deviceBuffer(BufferUsage::Uniform, 1), _command() {};
	virtual ~NullGraphicsSurface() = default;

	virtual void copyFrom(HostImage& hostImage) override {}
	virtual void copyTo(HostImage& hostImage) override {}

	virtual const DeviceBuffer& getDeviceImageData() const override { return _deviceBuffer; }

	virtual ImageFormat getFormat() const override { return ImageFormat::R8G8B8A8; }
	virtual glm::ivec2 getExtent() const override { return {-1, -1}; }
	virtual uint32_t getWidth() const override { return std::numeric_limits<uint32_t>::max(); }
	virtual uint32_t getHeight() const override { return std::numeric_limits<uint32_t>::max(); }

	virtual GraphicsCommand& beginCommandPass() override { return _command; }
	virtual void endCommandPass(GraphicsCommand& commandContext) override {}

	virtual void beginRenderPass(GraphicsCommand& commandContext) override {}
	virtual void endRenderPass(GraphicsCommand& commandContext) override {}

	virtual uint32_t getMultisampleCount() const override { return 1; }
	virtual bool getUseDynamicStates() const override { return false; }

	virtual void setExtent(const glm::ivec2& size) override {}
};

class NullRenderTarget : public RenderTarget
{
protected:
	NullDeviceBuffer _deviceBuffer;

public:
	NullRenderTarget(EngineContext& context) : RenderTarget(), _deviceBuffer(BufferUsage::Uniform, 1)
	{
		_primitiveFactory = std::make_unique<PrimitiveFactory>(context, *this);
	}
	virtual ~NullRenderTarget() = default;

	virtual void copyFrom(HostImage& hostImage) override {}
	virtual void copyTo(HostImage& hostImage) override {}

	virtual const DeviceBuffer& getDeviceImageData() const override { return _deviceBuffer; }
	
	virtual ImageFormat getFormat() const override { return ImageFormat::R8G8B8A8; }
	virtual glm::ivec2 getExtent() const override { return {-1, -1}; }
	virtual uint32_t getWidth() const override { return std::numeric_limits<uint32_t>::max(); }
	virtual uint32_t getHeight() const override { return std::numeric_limits<uint32_t>::max(); }

	virtual void beginRenderPass(GraphicsCommand& commandContext) override {}
	virtual void endRenderPass(GraphicsCommand& commandContext) override {}

	virtual uint32_t getMultisampleCount() const override { return 1; }
	virtual bool getUseDynamicStates() const override { return false; }

	virtual void setExtent(const glm::ivec2& size) override {}
};

class NullShader : public Shader
{
public:
	NullShader(const std::string& name) : Shader(name) {};
	virtual ~NullShader() = default;
};

class NullShaderManager : public ShaderManager
{
public:
	NullShaderManager() = default;
	virtual ~NullShaderManager() = default;

	// load shader from memory
	virtual std::unique_ptr<Shader> loadShaderFromMemory(const std::string& name, const std::string shader, ShaderType type = ShaderType::InferFromSource) override
	{
		return std::make_unique<NullShader>(name);
	}

	// load shader from file
	virtual std::unique_ptr<Shader> loadShaderFromFile(const std::string& name, const std::filesystem::path& path, ShaderType type = ShaderType::InferFromSource) override
	{
		return std::make_unique<NullShader>(name);
	}
};

class NullPipelineBinding : public PipelineBinding
{
public:
	NullPipelineBinding() : PipelineBinding() {};
	virtual ~NullPipelineBinding() = default;

	virtual void setVertexBuffer(DeviceBuffer& buffer) override {}
	virtual void setIndexBuffer(DeviceBuffer& buffer) override {}
	virtual void setUniformBuffer(ShaderStage stage, uint32_t binding, const DeviceBuffer& buffer) override {}

	virtual void setPushConstant(const SimpleRTTR::Type& type, ShaderStage stage, const void* data, std::size_t size) override {}

	virtual void setTexture(ShaderStage stage, uint32_t binding, const DeviceImage& image) override {}

	virtual void commit(GraphicsCommand& command) override {}
	virtual void commit(GraphicsCommand& command, size_t offset, size_t count) override {}
};

class NullPipeline : public Pipeline
{
public:
	NullPipeline() : Pipeline() {};
	virtual ~NullPipeline() = default;

	virtual std::unique_ptr<PipelineBinding> createBinding() override
	{
		return std::make_unique<NullPipelineBinding>();
	}
};

class NullPipelineManager : public PipelineManager
{
public:


	virtual std::unique_ptr<Pipeline> createPipeline(const PipelineDefinition& pipelineDefinition) override
	{
		return std::make_unique<NullPipeline>();
	}
};

class NullBufferManager : public BufferManager
{
public:
	NullBufferManager() = default;
	virtual ~NullBufferManager() = default;

	virtual std::unique_ptr<HostBuffer> createHostBuffer(std::size_t elementSize, std::size_t count, BufferUsage usage) override
	{
		return std::make_unique<NullHostBuffer>(usage, elementSize);
	}

	virtual std::unique_ptr<HostBuffer> createHostBuffer(DeviceBuffer& deviceBuffer) override
	{
		return std::make_unique<NullHostBuffer>(deviceBuffer.getUsage(), deviceBuffer.getElementSize());
	}

	virtual std::unique_ptr<DeviceBuffer> createDeviceBuffer(std::size_t elementSize, std::size_t count, BufferUsage usage) override
	{
		return std::make_unique<NullDeviceBuffer>(usage, elementSize);
	}

	virtual std::unique_ptr<DeviceBuffer> createDeviceBuffer(HostBuffer& hostBuffer) override
	{
		return std::make_unique<NullDeviceBuffer>(hostBuffer.getUsage() , hostBuffer.getElementSize());
	}
};

class NullImageManager : public ImageManager
{
protected:

public:
	NullImageManager() = default;
	virtual ~NullImageManager() = default;

	virtual OwningHandle<HostImage> createHostImage(const std::string& name, glm::ivec2 size, ImageFormat format) override
	{
		return _hostImageManager.add(std::make_unique<NullHostImage>(size, format));
	}

	virtual OwningHandle<DeviceImage> createDeviceImage(const std::string& name, glm::ivec2 size, ImageFormat format) override
	{
		return _deviceImageManager.add(std::make_unique<NullDeviceImage>(size, format));
	}

	virtual OwningHandle<DeviceImage> createDeviceImage(HostImage& host) override
	{
		return _deviceImageManager.add(std::make_unique<NullDeviceImage>(host.getExtent(), host.getFormat()));
	}
};

class NullGraphicsSystem : public GraphicsSystem
{
protected:
	EngineContext& _context;

public:
	NullGraphicsSystem(EngineContext& context) : _context(context) { };
	virtual ~NullGraphicsSystem() = default;

	virtual std::unique_ptr<GraphicsSurface> createWindowedSurface(PlatformWindow& window) override
	{
		return std::make_unique<NullGraphicsSurface>();
	}

	virtual std::unique_ptr<GraphicsSurface> createHeadlessSurface() override
	{
		return std::make_unique<NullGraphicsSurface>();
	}

	virtual std::unique_ptr<RenderTarget> createRenderTarget(glm::ivec2 size, ImageFormat format, glm::vec4 color) override
	{
		return std::make_unique<NullRenderTarget>(_context);
	}

	virtual std::unique_ptr<ShaderManager> createShaderManager() override
	{
		return std::make_unique<NullShaderManager>();
	}

	virtual std::unique_ptr<PipelineManager> createPipelineManager() override
	{
		return std::make_unique<NullPipelineManager>();
	}

	virtual std::unique_ptr<BufferManager> createBufferManager() override
	{
		return std::make_unique<NullBufferManager>();
	}

	virtual std::unique_ptr<ImageManager> createImageManager() override
	{
		return std::make_unique<NullImageManager>();
	}
};

} // namespace OpenXcom
