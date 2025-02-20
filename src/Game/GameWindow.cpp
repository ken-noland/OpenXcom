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
#include "GameWindow.h"
#include "../Engine/EngineContext.h"
#include "../Engine/Graphics/Common/WindowSurface.h"
#include "../Engine/Graphics/Common/GameSurface.h"
#include "../Engine/Graphics/GraphicsCommand.h"

#include <functional>

/////////////////////////////////////////////
// TEMP
#include "../Engine/Engine.h"
#include "../Engine/Platform/Window.h"
#include "../Engine/Resource/ResourceSystem.h"
#include "../Engine/Graphics/GraphicsSurface.h"
#include "../Engine/Graphics/Image/ImageManager.h"
#include "../Engine/Graphics/Palette/Palette.h"
#include "../Engine/Graphics/Palette/PaletteManager.h"
#include "../Engine/Graphics/Primitive/PrimitiveFactory.h"
#include "../Engine/Graphics/Primitive/LinePrimitive.h"
#include "../Engine/Graphics/Primitive/BoxPrimitive.h"
#include "../Engine/Graphics/Primitive/PointPrimitive.h"
#include "../Engine/Graphics/Primitive/ImagePrimitive.h"
#include "../Engine/Resource/FileProcessor/ImageFile.h"
#include "../Engine/Resource/FileProcessor/ImageBMPFileProcessor.h"

namespace OpenXcom
{

GameWindow::GameWindow(EngineContext& engine)
{
	Options& options = engine.getOptions();

	_gameSurface = std::make_unique<GameSurface>(engine);
	_windowSurface = std::make_unique<WindowSurface>(engine, *_gameSurface);

	_onWindowRender = _windowSurface->onRender().add([this](GraphicsCommand& command) {
		this->onWindowRender(command);
	});

	_onGameRender = _gameSurface->onRender().add([this](GraphicsCommand& command) {
		this->onGameRender(command);
	});
}

GameWindow::~GameWindow()
{
	_onWindowRender.reset();
	_onGameRender.reset();

	_windowSurface.reset();
	_gameSurface.reset();
}

void GameWindow::update()
{
	_windowSurface->update();
}

GameSurface& GameWindow::getGameSurface()
{
	return *_gameSurface;
}

WindowSurface& GameWindow::getWindowSurface()
{
	return *_windowSurface;
}

void GameWindow::setGameSurfaceSize(const glm::ivec2& size)
{
	_gameSurface->setScreenSize(size);
	_windowSurface->bindGameSurface(*_gameSurface);
}

bool GameWindow::isRunning() const
{
	return _windowSurface->isRunning();
}

void GameWindow::onWindowRender(GraphicsCommand& command)
{
}

void GameWindow::onGameRender(GraphicsCommand& command)
{
}

} // namespace OpenXcom
