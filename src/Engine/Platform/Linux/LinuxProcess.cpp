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

#include "../ProcessSystem.h"
#include <X11/Xlib.h>
#include <stdexcept>

namespace OpenXcom
{

void PlatformProcessSystem::platformSpecificUpdate()
{
	//if (this->displayHandle)
	//{
	//	XEvent event;
	//	while (XPending(static_cast<Display*>(this->displayHandle)))
	//	{
	//		XNextEvent(static_cast<Display*>(this->displayHandle), &event);

	//		switch (event.type)
	//		{
	//		case ClientMessage:
	//			// Handle window close event (e.g., "WM_DELETE_WINDOW")
	//			if (event.xclient.data.l[0] == this->wmDeleteMessage)
	//			{
	//				_isRunning = false; // Set flag to indicate window close
	//			}
	//			break;
	//		// Add other event handling cases as needed
	//		default:
	//			break;
	//		}
	//	}
	//}
}

void PlatformProcessSystem::platformSpecificExit()
{
	_isRunning = false; // Signal to Engine that the application should stop
}


} // namespace OpenXcom

#endif // __linux__
