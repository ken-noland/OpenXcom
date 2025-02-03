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
#include "../Engine/Graphics/Primitive/PointPrimitive.h"
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
		0x000000FF, // 0 - Black:   (R=00, G=00, B=00, A=FF)
		0xFFFFFFFF, // 1 - White:   (R=FF, G=FF, B=FF, A=FF)
		0x808080FF, // 2 - Gray:    (R=80, G=80, B=80, A=FF)
		0xFF0000FF, // 3 - Red:     (R=FF, G=00, B=00, A=FF)
		0x00FF00FF, // 4 - Green:   (R=00, G=FF, B=00, A=FF)
		0x0000FFFF, // 5 - Blue:    (R=00, G=00, B=FF, A=FF)
		0xFFFF00FF, // 6 - Yellow:  (R=FF, G=FF, B=00, A=FF)  // red + green
		0xFF00FFFF, // 7 - Magenta: (R=FF, G=00, B=FF, A=FF)  // red + blue
		0xCCCCCCFF, // 8 - Light Gray: (R=CC, G=CC, B=CC, A=FF)
		0x444444FF, // 9 - Dark Gray:  (R=44, G=44, B=44, A=FF)
		0xFFA500FF, // 10 - Orange:   (R=FF, G=A5, B=00, A=FF)  // red + part green
		0x800080FF, // 11 - Purple:   (R=80, G=00, B=80, A=FF)
		0xA54220FF, // 12 - Brown:    (R=A5, G=42, B=20, A=FF)
		0xFFC0CBFF, // 13 - Pink:     (R=FF, G=C0, B=CB, A=FF)
		0x008080FF, // 14 - Teal:     (R=00, G=80, B=80, A=FF)
		0xFFD400FF  // 15 - Gold:     (R=FF, G=D4, B=00, A=FF)
	};

	PaletteManager& paletteManager = engine.getResourceSystem().getPaletteManager();
	_paletteHandle = paletteManager.createPalette("16colors", paletteData, 16);

	// Create 16 vertices arranged in 2 rows (8 columns per row),
	// cycling through the 16 palette colors.
	std::vector<PointColorVertex> vertices;
	for (int row = 0; row < 2; ++row)
	{
		for (int col = 0; col < 8; ++col)
		{
			// Calculate the overall index and cycle through 16 colors.
			int index = row * 8 + col;
			int color = index % 16; // Cycle through colors 0-15

			// Create a vertex starting at (10,10) with 20 pixels spacing.
			PointColorVertex vertex = {glm::ivec2(10 + col * 20, 10 + row * 20), color};
			vertices.push_back(vertex);
		}
	}

	// Create the PointColorListPrimitive using the primitive factory.
	_thingToDraw = _gameSurface->getRenderTarget().getPrimitiveFactory().createPointColorListPrimitive(vertices.data(), vertices.size(), _paletteHandle.getHandle());
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
	_thingToDraw->draw(command);
}


} // namespace OpenXcom
