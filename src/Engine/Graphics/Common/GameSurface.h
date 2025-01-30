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
#include <memory>
#include "../../Utility/Delegate.h"
#include <glm/vec2.hpp>

namespace OpenXcom
{

class EngineContext;
class RenderTarget;
class HostImage;
class GraphicsCommand;
class Buffer;

class GameSurface
{
protected:
	std::unique_ptr<RenderTarget> _renderTarget;

	// events
	MulticastDelegate<void(GraphicsCommand&)> _onRender;

public:
	GameSurface(EngineContext& options);
	~GameSurface();

	void render(GraphicsCommand& command);

	void captureFrame(HostImage& image);

	const RenderTarget& getRenderTarget() const { return *_renderTarget; }
	glm::ivec2 getScreenSize() const;

	MulticastDelegate<void(GraphicsCommand&)>& onRender() { return _onRender; }
};

} // namespace OpenXcom
