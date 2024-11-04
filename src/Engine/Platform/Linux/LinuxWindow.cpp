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
#ifdef __linux__

#include "../Window.h"
#include "../../Engine.h"

#include <X11/Xlib.h>
#include <stdexcept>

namespace OpenXcom
{

void PlatformWindow::platformSpecificCreateWindow(const std::string& windowTitle, int width, int height)
{
	Display* display = XOpenDisplay(nullptr);
	if (!display)
	{
		throw std::runtime_error("Failed to open X display");
	}

	int screen = DefaultScreen(display);
	Window rootWindow = RootWindow(display, screen);

	Window window = XCreateSimpleWindow(
		display, rootWindow,
		10, 10, width, height,
		1, BlackPixel(display, screen), WhitePixel(display, screen));

	XStoreName(display, window, windowTitle.c_str());
	XSelectInput(display, window, ExposureMask | KeyPressMask);
	XMapWindow(display, window);

	Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, window, &wmDeleteMessage, 1);

	_handle.display = display;
	_handle.window = window;
	_handle.wmDeleteMessage = wmDeleteMessage;
}

void PlatformWindow::platformSpecificDestroyWindow()
{
	if (_handle.display && _handle.window)
	{
		XDestroyWindow(static_cast<Display*>(_handle.display), static_cast<Window>(_handle.window));
		XCloseDisplay(static_cast<Display*>(_handle.display));
		_handle.display = nullptr;
		_handle.window = 0;
	}
}

void PlatformWindow::platformSpecificUpdateWindow()
{
	if (_handle.display)
	{
		XEvent event;
		while (XPending(static_cast<Display*>(_handle.display)))
		{
			XNextEvent(static_cast<Display*>(_handle.display), &event);

			switch (event.type)
			{
			case ClientMessage:
				// Handle window close event (e.g., "WM_DELETE_WINDOW")
				if (event.xclient.data.l[0] == _handle.wmDeleteMessage)
				{
					//this->isRunning = false; // Set flag to indicate window close
					getEngine().exit();
				}
				break;
			// Add other event handling cases as needed
			default:
				break;
			}
		}
	}
}


} // namespace OpenXcom

#endif
