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
#include "../Engine/Resource/ResourceSystem.h"
#include "../Engine/Graphics/GraphicsSurface.h"
#include "../Engine/Graphics/Palette/Palette.h"
#include "../Engine/Graphics/Palette/PaletteManager.h"
#include "../Engine/Graphics/Primitive/PrimitiveFactory.h"
#include "../Engine/Graphics/Primitive/LinePrimitive.h"
#include "../Engine/Graphics/Primitive/BoxPrimitive.h"
/////////////////////////////////////////////


namespace OpenXcom
{

GameWindow::GameWindow(EngineContext& engine)
{
	Options& options = engine.getOptions();

	_gameSurface = std::make_unique<GameSurface>(engine);
	_windowSurface = std::make_unique<WindowSurface>(engine, *_gameSurface);

	_windowSurface->onRender() << [this](GraphicsCommand& command) {
		this->onWindowRender(command);
	};

	_gameSurface->onRender() << [this](GraphicsCommand& command) {
		this->onGameRender(command);
	};

	// set up the palette
	PackedColor paletteData[] = {
		0x00000000, // 0 - Black
		0xFFFFFFFF, // 1 - White
		0xFF808080, // 2 - Gray
		0xFFFF0000, // 3 - Red
		0xFF00FF00, // 4 - Green
		0xFF0000FF, // 5 - Blue
		0xFFFFFF00, // 6 - Yellow
		0xFFFF00FF, // 7 - Magenta
		0xFF00FFFF, // 8 - Cyan
		0xFFFFA500, // 9 - Orange
		0xFF8A2BE2, // 10 - Blue Violet
		0xFF008080, // 11 - Teal
		0xFF4B0082, // 12 - Indigo
		0xFF800000, // 13 - Maroon
		0xFF808000, // 14 - Olive
		0xFF8B4513  // 15 - Saddle Brown
	};

	PaletteManager& paletteManager = engine.getResourceSystem().getPaletteManager();
	_paletteHandle = paletteManager.createPalette("16colors", paletteData, 16);

	//LineVertex lines[] = {
	//	{{10, 10}},
	//	{{10, 190}},
	//	{{310, 190}},
	//	{{310, 10}},
	//	{{10, 10}}
	//};

	_box = _gameSurface->getRenderTarget().getPrimitiveFactory().createOutlineBoxPrimitive({10,10}, {300, 180}, 1, _paletteHandle.getHandle());
}

GameWindow::~GameWindow()
{
}

void GameWindow::update()
{
	_windowSurface->update();
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
	_box->draw(command);
}


} // namespace OpenXcom
