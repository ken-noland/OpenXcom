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
#include "ImagePrimitive.h"

#include "../Buffer/Buffer.h"
#include "../Buffer/BufferManager.h"
#include "../GraphicsCommand.h"
#include "../GraphicsSurface.h"
#include "../Palette/Palette.h"
#include "../Palette/PaletteManager.h"
#include "../Pipeline.h"
#include "../PipelineBinding.h"
#include "../PipelineDefinition.h"
#include "../Image/ImageManager.h"
#include "../Shader.h"

#include "../../EngineContext.h"
#include "../../Resource/ResourceSystem.h"

#include "../../Logger.h"

#include "../../Utility/RTTR.h"

SIMPLERTTR
{
	SimpleRTTR::registration().type<OpenXcom::ImageVertex>()
		.property(&OpenXcom::ImageVertex::pos, "pos")
		.property(&OpenXcom::ImageVertex::uv, "uv");
}

namespace OpenXcom
{

ImagePrimitive::ImagePrimitive(EngineContext& context, Pipeline& pipeline, RenderTarget& surface,
							   glm::ivec2 dstPosition, glm::ivec2 srcPosition, glm::ivec2 extents,
							   const ResourceHandle<DeviceImage>& imageHandle,
							   const ResourceHandle<Palette>& paletteHandle)
	: _context(context)
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	BufferManager& bufferManager = resourceSystem.getBufferManager();

	_vertexHostBuffer = bufferManager.createHostBuffer(sizeof(ImageVertex), 6, BufferUsage::Vertex);
	_vertexDeviceBuffer = bufferManager.createDeviceBuffer(sizeof(ImageVertex), 6, BufferUsage::Vertex);

	_pipelineBinding = pipeline.createBinding();

	set(dstPosition, srcPosition, extents);
	setImage(imageHandle);
	setPalette(paletteHandle);
	setSurface(surface);
}

ImagePrimitive::~ImagePrimitive()
{
}

void ImagePrimitive::set(glm::ivec2 dstPosition, glm::ivec2 srcPosition, glm::ivec2 extents)
{
	// Define two triangles that form a quad
	ImageVertex vertices[6] = {
		// Triangle 1
		{dstPosition, srcPosition},                                                       // Top-left
		{dstPosition + glm::ivec2(extents.x, 0), srcPosition + glm::ivec2(extents.x, 0)}, // Top-right
		{dstPosition + glm::ivec2(0, extents.y), srcPosition + glm::ivec2(0, extents.y)}, // Bottom-left

		// Triangle 2
		{dstPosition + glm::ivec2(extents.x, 0), srcPosition + glm::ivec2(extents.x, 0)}, // Top-right
		{dstPosition + extents, srcPosition + extents},                                   // Bottom-right
		{dstPosition + glm::ivec2(0, extents.y), srcPosition + glm::ivec2(0, extents.y)}  // Bottom-left
	};

	_vertexHostBuffer->copy(vertices, sizeof(vertices));
	_vertexDeviceBuffer->copy(*_vertexHostBuffer);

	_pipelineBinding->setVertexBuffer(*_vertexDeviceBuffer);
}

void ImagePrimitive::setPalette(const ResourceHandle<Palette>& paletteHandle)
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	PaletteManager& paletteManager = resourceSystem.getPaletteManager();
	Palette& palette = paletteManager.get(paletteHandle);

	_pipelineBinding->setUniformBuffer(ShaderStage::Fragment, 3, palette.getDeviceBuffer()); // bind the palette buffer
}

void ImagePrimitive::setImage(const ResourceHandle<DeviceImage>& imageHandle)
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	ImageManager& imageManager = resourceSystem.getImageManager();
	const DeviceImage& image = imageManager.getDeviceImageManager().get(imageHandle);

	_pipelineBinding->setUniformBuffer(ShaderStage::Vertex, 1, image.getDeviceImageData()); // bind the surface extents
	_pipelineBinding->setTexture(ShaderStage::Fragment, 2, image);                          // bind the image texture
}

void ImagePrimitive::setSurface(RenderTarget& surface)
{
	Log(LOG_DEBUG) << "ImagePrimitive::setSurface";
	Log(LOG_DEBUG) << "dimensions (width=" << surface.getExtent().x << ", height=" << surface.getExtent().y << " )";
	_pipelineBinding->setUniformBuffer(ShaderStage::Vertex, 0, surface.getDeviceImageData()); // bind the surface extents
}

void ImagePrimitive::draw(GraphicsCommand& command)
{
	_pipelineBinding->commit(command);
}

} // namespace OpenXcom
