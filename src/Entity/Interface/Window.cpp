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
#include "Window.h"
#include "../../fmath.h"
#include "../../Engine/Timer.h"
#include "../../Engine/RNG.h"
#include "../Engine/Tickable.h"
#include "../Engine/Drawable.h"

namespace OpenXcom
{

const double WindowComponent::POPUP_SPEED = 0.05;

WindowComponent::WindowComponent(SurfaceComponent& surfaceComponent, TickableComponent& tickableComponent,
	DrawableComponent& drawableComponent, State* state, WindowPopup popup)
	: _surfaceComponent(surfaceComponent), _bg(0), _color(0), _popup(popup), _popupStep(0.0),
	_state(state), _contrast(false), _screen(false), _thinBorder(false), _innerColor(0), _mute(false)
{
	drawableComponent.addDrawable(std::bind(&WindowComponent::draw, this));
	tickableComponent.addTickable(std::bind(&WindowComponent::tick, this));

	// the background image is always positioned at 0,0, so this sets the initial position of the background to be cropped
	_dx = -_surfaceComponent.getSurface()->getX();
	_dy = -_surfaceComponent.getSurface()->getY();

	_timer = new Timer(10);
	_timer->onSurface(std::bind(&WindowComponent::popup, this));

	if (_popup == WindowPopup::POPUP_NONE)
	{
		_popupStep = 1.0;
	}
	else
	{
		_surfaceComponent.getSurface()->setHidden(true);

		_timer->start();
		if (_state != 0)
		{
			_screen = state->isScreen();
			if (_screen)
				_state->toggleScreen();
		}
	}
}

WindowComponent::~WindowComponent()
{
	delete _timer;
}

/// Sets the background surface.
void WindowComponent::setBackground(const Surface* bg)
{
	_bg = bg;
	_surfaceComponent.getSurface()->setRedraw(true);
}

/// Sets the border color.
void WindowComponent::setColor(Uint8 color)
{
	_color = color;
	_surfaceComponent.getSurface()->setRedraw(true);
}

/// Gets the border color.
Uint8 WindowComponent::getColor() const
{
	return _color;
}

/// Sets the high contrast color setting.
void WindowComponent::setHighContrast(bool contrast)
{
	_contrast = contrast;
	_surfaceComponent.getSurface()->setRedraw(true);
}

/// Handles the timers.
void WindowComponent::tick()
{
	Surface* windowSurface = _surfaceComponent.getSurface();
	if (windowSurface->isHidden() && _popupStep < 1.0)
	{
		_state->hideAll();
		_surfaceComponent.getSurface()->setHidden(false);
	}

	_timer->think(false, true);
}

/// Popups the window.
void WindowComponent::popup()
{
	//if (!_mute && AreSame(_popupStep, 0.0))
	//{
	//	int sound = RNG::seedless(0, 2);
	//	if (soundPopup[sound] != 0)
	//	{
	//		soundPopup[sound]->play(Mix_GroupAvailable(0));
	//	}
	//}
	if (_popupStep < 1.0)
	{
		_popupStep += POPUP_SPEED;
	}
	else
	{
		if (_screen)
		{
			_state->toggleScreen();
		}
		_state->showAll();
		_popupStep = 1.0;
		_timer->stop();
	}

	Surface* windowSurface = _surfaceComponent.getSurface();
	windowSurface->setRedraw(true);
}

/// Draws the window.
void WindowComponent::draw()
{
	Surface* windowSurface = _surfaceComponent.getSurface();
	windowSurface->draw();

	SDL_Rect square;

	Sint16 width = windowSurface->getWidth();
	Sint16 height = windowSurface->getHeight();

	if (_popup == WindowPopup::POPUP_HORIZONTAL || _popup == WindowPopup::POPUP_BOTH)
	{
		square.x = (Sint16)((width - (width * _popupStep)) / 2);
		square.w = (Sint16)(width * _popupStep);
	}
	else
	{
		square.x = 0;
		square.w = width;
	}
	if (_popup == WindowPopup::POPUP_VERTICAL || _popup == WindowPopup::POPUP_BOTH)
	{
		square.y = (Sint16)((height - (height * _popupStep)) / 2);
		square.h = (Sint16)(height * _popupStep);
	}
	else
	{
		square.y = 0;
		square.h = height;
	}

	int mul = 1;
	if (_contrast)
	{
		mul = 2;
	}
	Uint8 color = _color + 3 * mul;

	if (_thinBorder)
	{
		color = _color + 1 * mul;
		for (int i = 0; i < 5; ++i)
		{
			windowSurface->drawRect(&square, color);

			if (i % 2 == 0)
			{
				square.x++;
				square.y++;
			}
			square.w--;
			square.h--;

			switch (i)
			{
			case 0:
				color = _color + 5 * mul;
				windowSurface->setPixel(square.w, 0, color);
				break;
			case 1:
				color = _color + 2 * mul;
				break;
			case 2:
				color = _color + 4 * mul;
				windowSurface->setPixel(square.w + 1, 1, color);
				break;
			case 3:
				color = _color + 3 * mul;
				break;
			}
		}
	}
	else
	{
		for (int i = 0; i < 5; ++i)
		{
			windowSurface->drawRect(&square, color);
			if (i < 2)
				color -= 1 * mul;
			else
				color += 1 * mul;
			square.x++;
			square.y++;
			if (square.w >= 2)
				square.w -= 2;
			else
				square.w = 1;

			if (square.h >= 2)
				square.h -= 2;
			else
				square.h = 1;
		}
		if (_innerColor != 0)
		{
			windowSurface->drawRect(&square, _innerColor);
		}
	}

	if (_bg != 0)
	{
		auto crop = _bg->getCrop();
		crop.getCrop()->x = square.x - _dx;
		crop.getCrop()->y = square.y - _dy;
		crop.getCrop()->w = square.w;
		crop.getCrop()->h = square.h;
		crop.setX(square.x);
		crop.setY(square.y);
		crop.blit(windowSurface);
	}
}

/// sets the X delta.
void WindowComponent::setDX(int dx)
{
	_dx = dx;
}

/// sets the Y delta.
void WindowComponent::setDY(int dy)
{
	_dy = dy;
}

/// Give this window a thin border.
void WindowComponent::setThinBorder()
{
	_thinBorder = true;
}

/// Give this window a custom inner color.
void WindowComponent::setInnerColor(Uint8 innerColor)
{
	_innerColor = innerColor;
}



} // namespace OpenXcom
