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
#include "../PlatformWindow.h"
#include "../../Engine.h"

#include <windows.h>

namespace OpenXcom
{

class Win32PlatformWindow : public PlatformWindow
{
protected:
	// little hack which allows access to protected members of PlatformWindow
	friend LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

// Custom window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Win32PlatformWindow* window = reinterpret_cast<Win32PlatformWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (!window)
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	switch (uMsg)
	{
	case WM_CLOSE:
		window->close();
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

	case WM_EXITSIZEMOVE:
	{
		// get the new size of the window
		RECT rect;
		GetClientRect(hwnd, &rect);
		window->resize(glm::ivec2(rect.right - rect.left, rect.bottom - rect.top));
		break;
	}

	case WM_SIZE:
	{
		// get the new size of the window
		RECT rect;
		GetClientRect(hwnd, &rect);
		glm::ivec2 newSize(rect.right - rect.left, rect.bottom - rect.top);
		if (wParam == SIZE_MINIMIZED)
		{
			window->_minimized = true;
			window->resize(newSize);
		}
		else if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED)
		{
			window->_minimized = false;
			window->resize(newSize);
		}
		break;
	}

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

	SetWindowLongPtr(_handle.hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

void PlatformWindow::platformSpecificDestroyWindow()
{
	if (_handle.hWnd != NULL)
	{
		DestroyWindow(_handle.hWnd);
		_handle.hWnd = NULL;
	}
}

void PlatformWindow::platformSpecificUpdateWindow()
{
	// nothing to see here since the window message loop is in the process system
}

void PlatformWindow::platformSpecificShowWindow()
{
	ShowWindow(_handle.hWnd, SW_SHOW);
}

void PlatformWindow::platformSpecificHideWindow()
{
	ShowWindow(_handle.hWnd, SW_HIDE);
}

} // namespace OpenXcom

#endif // defined(_WIN32) || defined(_WIN64)


/////////////////////////////////////////////////////

//
///**
// * Sets the window titlebar icon.
// * For Windows, use the embedded resource icon.
// * For other systems, use a PNG icon.
// * @param winResource ID for Windows icon.
// * @param unixPath Path to PNG icon for Unix.
// */
//#ifdef _WIN32
//void setWindowIcon(int winResource, const std::string&)
//{
//	assert(!"Not implemented");
//	// SDLHACK
//	// HINSTANCE handle = GetModuleHandle(NULL);
//	// HICON icon = LoadIcon(handle, MAKEINTRESOURCE(winResource));
//
//	// SDL_SysWMinfo wminfo;
//	// SDL_VERSION(&wminfo.version)
//	// if (SDL_GetWMInfo(&wminfo))
//	//{
//	//	HWND hwnd = wminfo.window;
//	//	SetClassLongPtr(hwnd, GCLP_HICON, (LONG_PTR)icon);
//	// }
//}
//#else
//void setWindowIcon(int, const std::string& unixPath)
//{
//	SDL_Surface* icon = IMG_Load_RW(FileMap::getRWops(unixPath), SDL_TRUE);
//	if (icon != 0)
//	{
//		SDL_WM_SetIcon(icon, NULL);
//		SDL_FreeSurface(icon);
//	}
//}
//#endif
