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
#if defined(_WIN32)
#include "../Window.h"

#include <windows.h>

namespace OpenXcom
{


// Custom window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0); // Post a WM_QUIT message to terminate the message loop
		return 0;
	case WM_KEYDOWN:
		// Handle key down events if needed
		break;
	case WM_KEYUP:
		// Handle key up events if needed
		break;
	case WM_MOUSEMOVE:
		// Handle mouse movement if needed
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		// Handle mouse button events if needed
		break;
	// Handle other messages as needed
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

void PlatformWindow::platformSpecificCreateWindow(const std::string& title, int width, int height)
{
	_handle.hInstance = GetModuleHandle(nullptr);
	WNDCLASSA wc = {0};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = _handle.hInstance;
	wc.lpszClassName = "CrossPlatformWindow";

	RegisterClassA(&wc);

	_handle.hWnd = CreateWindowExA(
		0, wc.lpszClassName, title.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		width, height, nullptr, nullptr, _handle.hInstance, nullptr);

	ShowWindow(_handle.hWnd, SW_SHOW);
}

void PlatformWindow::platformSpecificClose()
{
	if (_handle.hWnd != NULL)
	{
		DestroyWindow(static_cast<HWND>(_handle.hWnd));
		_handle.hWnd = NULL;
	}
}

} // namespace OpenXcom

#endif // defined(_WIN32) || defined(_WIN64)
