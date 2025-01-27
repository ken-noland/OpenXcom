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
#include "../Engine/Graphics/Common/WindowSurface.h"
#include "../Engine/Graphics/Common/GameSurface.h"

namespace OpenXcom
{

GameWindow::GameWindow(const std::string& title, Options& options)
{
	_gameSurface = std::make_unique<GameSurface>(options);
	_windowSurface = std::make_unique<WindowSurface>(title, options, *_gameSurface);
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

} // namespace OpenXcom
