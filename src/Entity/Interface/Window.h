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

namespace OpenXcom
{

class State;
class Surface;

class Sound;

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
struct WindowComponent
{
	WindowComponent(WindowPopup popup) : _bg(nullptr), _color(0), _popup(popup), _popupStep(0.0), _state(nullptr), _contrast(false), _screen(false), _thinBorder(false), _innerColor(0), _mute(false) {}

	const Surface* _bg;
	uint8_t _color;
	WindowPopup _popup;
	double _popupStep;
	State* _state;
	bool _contrast, _screen, _thinBorder;
	uint8_t _innerColor;
	bool _mute;
};

/**
 * The window background component
 */
struct BackgroundComponent
{
	BackgroundComponent(const Surface* bg) : _dx(0), _dy(0), _bg(bg) {}
	int _dx, _dy;
	const Surface* _bg;
};


class WindowSystem
{
protected:
	//KN Note: sure would be nice to move sounds over to handles!
	std::vector<Sound*> soundPopup;

public:
	WindowSystem();
	~WindowSystem();

	static const uint64_t POPUP_SPEED_MS;

	/// Sets the background surface.
	[[deprecated]] void setBackground(entt::handle windowEntity, const Surface* bg);

	/// Sets the border color.
	void setColor(entt::handle windowEntity, uint8_t color);
	/// Gets the border color.
	uint8_t getColor(entt::handle windowEntity) const;
	/// Sets the high contrast color setting.
	void setHighContrast(entt::handle windowEntity, bool contrast);

	/// Draw the window
	void draw(entt::handle windowEntity);

	/// sets the X delta.
	void setDX(entt::handle windowEntity, int dx);

	/// sets the Y delta.
	void setDY(entt::handle windowEntity, int dy);

	/// Give this window a thin border.
	void setThinBorder(entt::handle windowEntity);

	/// Give this window a custom inner color.
	void setInnerColor(entt::handle windowEntity, uint8_t innerColor);

	/// add a sound to the available popup sounds
	void addPopupSound(Sound* sound);

	/// Play the popup sound.
	void playSound(entt::handle windowEntity);

	/// Mute the window.
	[[deprecated]] void setMute(entt::handle windowEntity, bool mute);

	/// Update the window reveal progress. (called from the ProgressTimerComponent)
	void UpdateProgress(entt::handle windowEntity, double progress);
	/// Finalize the window reveal progress. (called from the ProgressTimerComponent)
	void CompleteProgress(entt::handle windowEntity);

};

} // namespace OpenXcom
