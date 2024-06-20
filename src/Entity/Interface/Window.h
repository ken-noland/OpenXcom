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
#include <entt/entt.hpp>
#include "../Engine/Surface.h"

namespace OpenXcom
{

class State;
class Timer;

/**
 * Enumeration for the type of animation when a window pops up.
 */
enum class WindowPopup
{
	POPUP_NONE,
	POPUP_HORIZONTAL,
	POPUP_VERTICAL,
	POPUP_BOTH
};


/**
 * The window component
 */
class WindowComponent
{
protected:
	SurfaceComponent& _surfaceComponent;

	static const double POPUP_SPEED;
	int _dx, _dy;
	const Surface* _bg;
	Uint8 _color;
	WindowPopup _popup;
	double _popupStep;
	Timer* _timer;
	State* _state;
	bool _contrast, _screen, _thinBorder;
	Uint8 _innerColor;
	bool _mute;

public:
	WindowComponent(SurfaceComponent& surfaceComponent, State* state, WindowPopup popup);
	~WindowComponent();

	/// Sets the background surface.
	[[deprecated]] void setBackground(const Surface* bg);
	/// Sets the border color.
	void setColor(Uint8 color);
	/// Gets the border color.
	Uint8 getColor() const;
	/// Sets the high contrast color setting.
	void setHighContrast(bool contrast);
	/// Handles the timers.
	void think();
	/// Popups the window.
	void popup();
	/// Draws the window.
	void draw();
	/// sets the X delta.
	void setDX(int dx);
	/// sets the Y delta.
	void setDY(int dy);
	/// Give this window a thin border.
	void setThinBorder();
	/// Give this window a custom inner color.
	void setInnerColor(Uint8 innerColor);
	/// Mute the window.
	void mute() { _mute = true; }
};

} // namespace OpenXcom
