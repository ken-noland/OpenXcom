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

#include <SDL_mixer.h>

#include "../../Engine/Timer.h"
#include "../../Engine/RNG.h"
#include "../Engine/Tickable.h"
#include "../Engine/Drawable.h"
#include "../Engine/Surface.h"
#include "../Engine/Hierarchical.h"

#include "../../Engine/Game.h"
#include "../../Engine/Screen.h"
#include "../../Engine/Sound.h"

namespace OpenXcom
{

const uint64_t WindowSystem::POPUP_SPEED_MS = 200;


WindowSystem::WindowSystem()
{
}

WindowSystem::~WindowSystem()
{
}

void WindowSystem::setBackground(entt::handle windowEntity, const Surface* bg)
{
	WindowComponent& windowComponent = windowEntity.get<WindowComponent>();
	windowComponent._bg = bg;
}

void WindowSystem::setColor(entt::handle windowEntity, uint8_t color)
{
	WindowComponent& windowComponent = windowEntity.get<WindowComponent>();
	windowComponent._color = color;
}

uint8_t WindowSystem::getColor(entt::handle windowEntity) const
{
	const WindowComponent& windowComponent = windowEntity.get<WindowComponent>();
	return windowComponent._color;
}

void WindowSystem::setHighContrast(entt::handle windowEntity, bool contrast)
{
	WindowComponent& windowComponent = windowEntity.get<WindowComponent>();
	windowComponent._contrast = contrast;
}

void WindowSystem::draw(entt::handle windowEntity)
{
	WindowComponent& windowComponent = windowEntity.get<WindowComponent>();
	SurfaceComponent& surfaceComponent = windowEntity.get<SurfaceComponent>();
	ProgressTimerComponent& progressTimerComponent = windowEntity.get<ProgressTimerComponent>();

	Surface* windowSurface = surfaceComponent.getSurface();

	windowSurface->draw();
	
	SDL_Rect square;
	
	Sint16 width = windowSurface->getWidth();
	Sint16 height = windowSurface->getHeight();
	
	if (windowComponent._popup == WindowPopup::POPUP_HORIZONTAL || windowComponent._popup == WindowPopup::POPUP_BOTH)
	{
		square.x = (Sint16)((width - (width * windowComponent._popupStep)) / 2);
		square.w = (Sint16)(width * windowComponent._popupStep);
	}
	else
	{
		square.x = 0;
		square.w = width;
	}
	if (windowComponent._popup == WindowPopup::POPUP_VERTICAL || windowComponent._popup == WindowPopup::POPUP_BOTH)
	{
		square.y = (Sint16)((height - (height * windowComponent._popupStep)) / 2);
		square.h = (Sint16)(height * windowComponent._popupStep);
	}
	else
	{
		square.y = 0;
		square.h = height;
	}
	
	int mul = 1;
	if (windowComponent._contrast)
	{
		mul = 2;
	}
	Uint8 color = windowComponent._color + 3 * mul;
	
	if (windowComponent._thinBorder)
	{
		color = windowComponent._color + 1 * mul;
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
				color = windowComponent._color + 5 * mul;
				windowSurface->setPixel(square.w, 0, color);
				break;
			case 1:
				color = windowComponent._color + 2 * mul;
				break;
			case 2:
				color = windowComponent._color + 4 * mul;
				windowSurface->setPixel(square.w + 1, 1, color);
				break;
			case 3:
				color = windowComponent._color + 3 * mul;
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
		if (windowComponent._innerColor != 0)
		{
			windowSurface->drawRect(&square, windowComponent._innerColor);
		}
	}
	
	if (windowComponent._bg != 0)
	{
		SurfaceCrop crop = windowComponent._bg->getCrop();
		crop.getCrop()->x = square.x - windowComponent._dx;
		crop.getCrop()->y = square.y - windowComponent._dy;
		crop.getCrop()->w = square.w;
		crop.getCrop()->h = square.h;
		crop.setX(square.x);
		crop.setY(square.y);
		crop.blit(windowSurface);
	}

	if (windowComponent._popupStep >= 1.0)
	{
		// Now render the window contents
		HierarchySystem& hierarchySystem = getSystem<HierarchySystem>();
		hierarchySystem.visit(windowEntity, [&](entt::handle child) {
			DrawableSystem& drawableSystem = getSystem<DrawableSystem>();
			drawableSystem.draw(child);
		});
	}


	SDL_Rect target{};
	target.x = surfaceComponent.getX();
	target.y = surfaceComponent.getY();
	SDL_BlitSurface(windowSurface->getSDLSurface(), nullptr, getGame()->getScreen()->getSurface(), &target);
}

void WindowSystem::setDX(entt::handle windowEntity, int dx)
{
	WindowComponent& windowComponent = windowEntity.get<WindowComponent>();
	windowComponent._dx = dx;
}

void WindowSystem::setDY(entt::handle windowEntity, int dy)
{
	WindowComponent& windowComponent = windowEntity.get<WindowComponent>();
	windowComponent._dy = dy;
}

void WindowSystem::setThinBorder(entt::handle windowEntity)
{
	WindowComponent& windowComponent = windowEntity.get<WindowComponent>();
	windowComponent._thinBorder = true;
}

void WindowSystem::setInnerColor(entt::handle windowEntity, uint8_t innerColor)
{
	WindowComponent& windowComponent = windowEntity.get<WindowComponent>();
	windowComponent._innerColor = innerColor;
}

void WindowSystem::playSound(entt::handle windowEntity)
{
	WindowComponent& windowComponent = windowEntity.get<WindowComponent>();
	if (!windowComponent._mute && soundPopup.size() > 0)
	{
		int sound = RNG::seedless(0, (int)soundPopup.size()-1);
		if (soundPopup[sound] != 0)
		{
			soundPopup[sound]->play(Mix_GroupAvailable(0));
		}
	}
}

void WindowSystem::setMute(entt::handle windowEntity, bool mute)
{
	WindowComponent& windowComponent = windowEntity.get<WindowComponent>();
	windowComponent._mute = mute;
}

void WindowSystem::UpdateProgress(entt::handle windowEntity, double progress)
{
	WindowComponent& windowComponent = windowEntity.get<WindowComponent>();
	windowComponent._popupStep = progress;
}

void WindowSystem::CompleteProgress(entt::handle windowEntity)
{
	WindowComponent& windowComponent = windowEntity.get<WindowComponent>();
	windowComponent._popupStep = 1.0;
}

void WindowSystem::addPopupSound(Sound* sound)
{
	soundPopup.push_back(sound);
}

} // namespace OpenXcom
