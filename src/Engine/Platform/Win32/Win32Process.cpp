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

#if defined(_WIN32) || defined(_WIN64)

#include "../ProcessSystem.h"
#include <windows.h>

namespace OpenXcom
{

void PlatformProcessSystem::platformSpecificUpdate()
{
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			_isRunning = false; // Signal to Engine that the application should stop
			return;      // Exit processing after handling WM_QUIT
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void PlatformProcessSystem::platformSpecificExit(int exitCode)
{
	// Signal to Engine that the application should stop
	_isRunning = false; 

	// Post a quit message to the message queue which will get picked up by the main loop(see above)
	PostQuitMessage(exitCode);
}

} // namespace OpenXcom

#endif // defined(_WIN32) || defined(_WIN64)
