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

#include "WindowSystem.h"
#include "Window.h"

namespace OpenXcom
{

PlatformWindowSystem::PlatformWindowSystem()
{
}

PlatformWindowSystem::~PlatformWindowSystem()
{
}

std::weak_ptr<PlatformWindow> PlatformWindowSystem::createWindow(const std::string& title, int width, int height)
{
	std::shared_ptr<PlatformWindow> window = std::make_shared<PlatformWindow>(title, width, height);
	_windows.push_back(window);
	return window;
}

void PlatformWindowSystem::destroyWindow(const std::weak_ptr<PlatformWindow>& window)
{
	std::shared_ptr<PlatformWindow> windowPtr = window.lock();
	auto it = std::find_if(_windows.begin(), _windows.end(), [&](const std::shared_ptr<PlatformWindow>& w) { return w == windowPtr; });
	if (it != _windows.end())
	{
		_windows.erase(it);
	}
}

void PlatformWindowSystem::update()
{
	if(_windows.size() == 0)
	{
		// signal the game loop that we have no windows open and we should shut down the game.
		//assert(!"Not implemented yet");
		return;
	}

	for (std::shared_ptr<PlatformWindow>& window : _windows)
	{
		window->update();
	}
}

} // namespace OpenXcom
