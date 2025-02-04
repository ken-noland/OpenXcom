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

#include "../Buffer.h"
#include "../BufferManager.h"
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

#include "../../Utility/RTTR.h"

SIMPLERTTR
{
	SimpleRTTR::registration().type<OpenXcom::ImageVertex>()
		.property(&OpenXcom::ImageVertex::pos, "pos")
		.property(&OpenXcom::ImageVertex::uv, "uv");
}

namespace OpenXcom
{

ImagePrimitive::ImagePrimitive(EngineContext& context, Pipeline& pipeline, RenderTarget& surface, glm
	::ivec2 dst, glm::ivec2 src, glm::ivec2 extents,
	const ResourceHandle<DeviceImage>& imageHandle, const ResourceHandle<Palette>& paletteHandle)
{
	ResourceSystem& resourceSystem = context.getResourceSystem();
	BufferManager& bufferManager = resourceSystem.getBufferManager();
	PaletteManager& paletteManager = resourceSystem.getPaletteManager();

	Palette& palette = paletteManager.get(paletteHandle);

	_pipelineBinding = pipeline.createBinding();

	// Define two triangles that form a quad
	ImageVertex vertices[6] = {
		// Triangle 1
		{dst, src},                                                       // Top-left
		{dst + glm::ivec2(extents.x, 0), src + glm::ivec2(extents.x, 0)}, // Top-right
		{dst + glm::ivec2(0, extents.y), src + glm::ivec2(0, extents.y)}, // Bottom-left

		// Triangle 2
		{dst + glm::ivec2(extents.x, 0), src + glm::ivec2(extents.x, 0)}, // Top-right
		{dst + extents, src + extents},                                   // Bottom-right
		{dst + glm::ivec2(0, extents.y), src + glm::ivec2(0, extents.y)}  // Bottom-left
	};

	_vertexBuffer = bufferManager.createDeviceBuffer<ImageVertex>(vertices, 6, BufferUsage::Vertex);
	_pipelineBinding->setVertexBuffer(*_vertexBuffer);

	ImageManager& imageManager = resourceSystem.getImageManager();
	const DeviceImage& image = imageManager.getDeviceImageManager().get(imageHandle);

	_pipelineBinding->setUniformBuffer(ShaderStage::Vertex, 0, surface.getDeviceImageData()); // bind the surface extents
	_pipelineBinding->setUniformBuffer(ShaderStage::Vertex, 1, image.getDeviceImageData()); // bind the surface extents

	_pipelineBinding->setTexture(ShaderStage::Fragment, 2, image);                           // bind the image texture
	_pipelineBinding->setUniformBuffer(ShaderStage::Fragment, 3, palette.getDeviceBuffer()); // bind the palette buffer
}

ImagePrimitive::~ImagePrimitive()
{
}

void ImagePrimitive::draw(GraphicsCommand& command)
{
	_pipelineBinding->commit(command);
}

} // namespace OpenXcom
