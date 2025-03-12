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
#include "Image/Image.h"
#include "Primitive/PrimitiveFactory.h"

namespace OpenXcom
{

class PrimitiveFactory;
class GraphicsSurface;
class GraphicsCommand;
class DeviceBuffer;

class RenderTarget : public DeviceImage
{
protected:
	std::unique_ptr<PrimitiveFactory> _primitiveFactory;

public:
	RenderTarget(ImageType imageType = ImageType::RenderTarget) : DeviceImage(imageType, "RenderTarget") {}
	virtual ~RenderTarget() = default;

	virtual void beginRenderPass(GraphicsCommand& commandContext) = 0;
	virtual void endRenderPass(GraphicsCommand& commandContext) = 0;

	virtual uint32_t getMultisampleCount() const = 0;
	virtual bool getUseDynamicStates() const = 0;

	// Warning: This function invalidates all primitives created from this surface
	virtual void setExtent(const glm::ivec2& size) = 0;

	// device buffer for a render target contains the extents of the render target
	virtual const DeviceBuffer& getDeviceImageData() const = 0;

	PrimitiveFactory& getPrimitiveFactory() { return *_primitiveFactory; }
};

class GraphicsSurface : public RenderTarget
{
public:
	GraphicsSurface() : RenderTarget(ImageType::Surface) {};
	virtual ~GraphicsSurface() = default;
		
	virtual GraphicsCommand& beginCommandPass() = 0;
	virtual void endCommandPass(GraphicsCommand& commandContext) = 0;
};

} // namespace OpenXcom
