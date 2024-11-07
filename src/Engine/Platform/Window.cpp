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

#include "Window.h"
#include "../Engine.h"
#include "../Graphics/GraphicsSystem.h"
#include "../Graphics/GraphicsInterfaces.h"

namespace OpenXcom
{

PlatformWindow::PlatformWindow(const std::string& title, int width, int height)
{
	platformSpecificCreateWindow(title, width, height);

	_running = true;

	_surface = getEngine().getGraphicsSystem().createSurface(_handle);
}

PlatformWindow::~PlatformWindow()
{
	platformSpecificDestroyWindow();
}

void PlatformWindow::update()
{
	platformSpecificUpdateWindow();

	_surface->update();
}

bool PlatformWindow::isRunning() const
{
	return _running;
}

void PlatformWindow::close()
{
	_running = false;

	// Destroy the surface before the window
	_surface.reset();

	platformSpecificDestroyWindow();
}

} // namespace OpenXcom
