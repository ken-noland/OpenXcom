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

#include <string>
#include <memory>

#if defined(_WIN32)
 // Forward declarations for Windows types
typedef struct HWND__* HWND;
typedef struct HINSTANCE__* HINSTANCE;
#elif defined(__linux__)
typedef unsigned long Atom;
#endif

namespace OpenXcom
{

class PlatformWindowSystem;
class GraphicsSurface;

struct PlatformWindowHandle
{
#if defined(_WIN32)
	HINSTANCE hInstance;
	HWND hWnd;
#elif defined(__linux__)
	void* display;        // Could be a pointer to X11 Display or Wayland connection.
	unsigned long window; // Window handle (X11) or equivalent (Wayland).

	// X11 specific
	Atom wmDeleteMessage;
#elif defined(__APPLE__)
	void* window; // NSWindow* equivalent
#elif defined(__ANDROID__)
	void* app; // NativeActivity pointer
#endif
};

class PlatformWindow
{
private:
	// These are defined here, but implemented in the platform-specific files
	void platformSpecificCreateWindow(const std::string& title, int width, int height);
	void platformSpecificDestroyWindow();
	void platformSpecificUpdateWindow();

	void platformSpecificShowWindow();
	void platformSpecificHideWindow();

	bool _running;

	PlatformWindowHandle _handle;

public:
	PlatformWindow(const std::string& title, int width, int height);
	~PlatformWindow();

	void update();

	bool isRunning() const;
	void close();

	void show() { platformSpecificShowWindow(); }
	void hide() { platformSpecificHideWindow(); }

	const PlatformWindowHandle& getHandle() { return _handle; }
};


} // namespace OpenXcom
