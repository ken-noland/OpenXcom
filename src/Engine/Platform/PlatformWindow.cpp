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

#include "PlatformWindow.h"
#include "../Engine.h"
#include "../Options.h"

#include <cassert>

namespace OpenXcom
{

PlatformWindow::PlatformWindow(const std::string& title, int width, int height)
{
	platformSpecificCreateWindow(title, width, height);

	_running = true;
	_minimized = false;
	_maximized = false;
	_restored = false;
}

PlatformWindow::~PlatformWindow()
{
	platformSpecificDestroyWindow();
}

void PlatformWindow::update()
{
	platformSpecificUpdateWindow();
}

bool PlatformWindow::isRunning() const
{
	return _running;
}

bool PlatformWindow::isMinimized() const
{
	return _minimized;
}


void PlatformWindow::close()
{
	_running = false;

	// run the callbacks before the platform window is called(in case the caller wants to override the close behavior)
	_onClose.call();

	platformSpecificDestroyWindow();
}

void PlatformWindow::resize(glm::ivec2 size)
{
	_onResize.call(size);
}

} // namespace OpenXcom
