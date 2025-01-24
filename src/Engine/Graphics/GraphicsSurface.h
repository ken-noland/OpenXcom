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
#include "../Resource/Image/Image.h"

namespace OpenXcom
{

class GraphicsSurface;
class GraphicsCommand;

class RenderTarget : public Image
{
public:
	RenderTarget() = default;
	virtual ~RenderTarget() = default;

	virtual void beginRenderPass(GraphicsCommand& commandContext) = 0;
	virtual void endRenderPass(GraphicsCommand& commandContext) = 0;
};

class GraphicsSurface : public RenderTarget
{
public:
	GraphicsSurface() = default;
	virtual ~GraphicsSurface() = default;
		
	virtual GraphicsCommand& beginCommandPass() = 0;
	virtual void endCommandPass(GraphicsCommand& commandContext) = 0;
};

} // namespace OpenXcom
