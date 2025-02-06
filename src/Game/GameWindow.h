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
#include <string>
#include <glm/vec2.hpp>

/////////////////////////////////////////////
// TEMP
#include "../Engine/Resource/Handle.h"

/////////////////////////////////////////////


namespace OpenXcom
{

class EngineContext;
class GameSurface;
class WindowSurface;
class GraphicsCommand;

class BoxOutlinePrimitive;
class LineStripPrimitive;
class PointColorListPrimitive;
class ImagePrimitive;

class Palette;
class DeviceImage;

class GameWindow
{
protected:
	std::unique_ptr<GameSurface> _gameSurface;
	std::unique_ptr<WindowSurface> _windowSurface;

	OwningHandle<Palette> _paletteHandle;
	OwningHandle<DeviceImage> _image;
	std::unique_ptr<ImagePrimitive> _thingToDraw;
	std::unique_ptr<BoxOutlinePrimitive> _thingToDraw2;

	void onWindowRender(GraphicsCommand& command);
	void onGameRender(GraphicsCommand& command);

	void onWindowResize(glm::ivec2);

public:
	GameWindow(EngineContext& engine);
	~GameWindow();

	void update();

	bool isRunning() const;
};

} // namespace OpenXcom
