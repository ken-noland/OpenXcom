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
#include "GameSurface.h"
#include "../GraphicsSystem.h"
#include "../GraphicsSurface.h"
#include "../GraphicsCommand.h"
#include "../../Engine.h"
#include "../../Resource/Image/Image.h"

namespace OpenXcom
{

GameSurface::GameSurface(Options& options)
{
	Engine& engine = getEngine();
	GraphicsSystem& graphicsSystem = engine.getGraphicsSystem();

	// create a render target for the game surface
	_renderTarget = graphicsSystem.createRenderTarget(320, 200, ImageFormat::RGBA8); // TODO: use game options to define the game surface dimensions
}

GameSurface::~GameSurface()
{
}

void GameSurface::render(GraphicsCommand& command)
{
	// Render the game surface
	command.beginRenderPass(*_renderTarget);

	// dispatch any rendering commands here
	_onRender.call(command);

	command.endRenderPass();
}

glm::ivec2 GameSurface::getScreenSize() const
{
	return _renderTarget->getSize();
}

} // namespace OpenXcom
