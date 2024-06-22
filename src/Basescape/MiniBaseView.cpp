/*
 * Copyright 2010-2024 OpenXcom Developers.
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
#include "MiniBaseView.h"
#include <cmath>
#include <ranges>
#include "../Engine/Action.h"
#include "../Engine/Options.h"
#include "../Engine/SurfaceSet.h"
#include "../Entity/Game/BasescapeData.h"
#include "../Savegame/Base.h"
#include "../Savegame/BaseFacility.h"
#include "../Mod/RuleBaseFacility.h"

namespace OpenXcom
{

/**
 * Sets up a mini base view with the specified size and position.
 * @param width Width in pixels.
 * @param height Height in pixels.
 * @param x X position in pixels.
 * @param y Y position in pixels.
 */
MiniBaseView::MiniBaseView(BasescapeSystem& basescapeSystem, int width, int height, int x, int y)
	: InteractiveSurface(width, height, x, y), _basescapeSystem(basescapeSystem)
{
	_unsubscribeId = _basescapeSystem.connectListener(std::bind_front(&MiniBaseView::draw, this));

	_baseKeys = {
		Options::keyBaseSelect1,
		Options::keyBaseSelect2,
		Options::keyBaseSelect3,
		Options::keyBaseSelect4,
		Options::keyBaseSelect5,
		Options::keyBaseSelect6,
		Options::keyBaseSelect7,
		Options::keyBaseSelect8
	};
}

MiniBaseView::~MiniBaseView()
{
	_basescapeSystem.disconnectListener(_unsubscribeId);
}

/**
 * Draws the view of all the bases with facilities
 * in varying colors.
 */
void MiniBaseView::draw()
{
	Surface::draw();

	size_t selectedBaseIndex = _basescapeSystem.getBasescapeData().getSelectedBaseVisibleIndex();
	Sint16 index = 0;
	for (entt::handle baseHandle : _basescapeSystem.getVisibleBases())
	{
		// Draw base squares
		if (index == selectedBaseIndex)
		{
			SDL_Rect r;
			r.x = index * (MINI_SIZE + 2);
			r.y = 0;
			r.w = MINI_SIZE + 2;
			r.h = MINI_SIZE + 2;
			drawRect(&r, 1);
		}
		_texture->getFrame(41)->blitNShade(this, index * (MINI_SIZE + 2), 0);

		// Draw facilities
		SDL_Rect r;
		lock();
		for (const BaseFacility* fac : baseHandle.get<Base>().getFacilities())
		{
			int color;
			if (fac->getDisabled())
				color = _blue;
			else if (fac->getBuildTime() == 0)
				color = _green;
			else
				color = _red;

			r.x = index * (MINI_SIZE + 2) + 2 + fac->getX() * 2;
			r.y = 2 + fac->getY() * 2;
			r.w = fac->getRules()->getSizeX() * 2;
			r.h = fac->getRules()->getSizeY() * 2;
			drawRect(&r, color+3);
			r.x++;
			r.y++;
			r.w--;
			r.h--;
			drawRect(&r, color+5);
			r.x--;
			r.y--;
			drawRect(&r, color+2);
			r.x++;
			r.y++;
			r.w--;
			r.h--;
			drawRect(&r, color+3);
			r.x--;
			r.y--;
			setPixel(r.x, r.y, color+1);
		}
		unlock();
		++index;
	}
}

void MiniBaseView::mouseClick(Action* action, State* state)
{
	size_t mouseIndex = static_cast<size_t>(floor(action->getRelativeXMouse() / ((MINI_SIZE + 2) * action->getXScale())));
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		if (_basescapeSystem.trySetSelectedBaseByVisbleIndex(mouseIndex)) { _redraw = true; }
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		if (mouseIndex == BasescapeData::MAX_VISIBLE_BASES - 1) { _basescapeSystem.tryIncrementVisibleBaseOffset(); }
		else if (mouseIndex == 0) { _basescapeSystem.tryDecrementVisibleBaseOffset(); }
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_MIDDLE)
	{
		// TODO: Implement the swapping behavior
	}
	InteractiveSurface::mouseClick(action, state);
}

void MiniBaseView::keyboardPress(Action* action, State* state)
{
	if (action->getDetails()->type == SDL_KEYDOWN)
	{
		SDLKey pressedKey = action->getDetails()->key.keysym.sym;

		auto findResult = std::ranges::find(_baseKeys, pressedKey);
		if (findResult != _baseKeys.end())
		{
			size_t index = std::distance(_baseKeys.begin(), findResult);
			if (_basescapeSystem.trySetSelectedBaseByVisbleIndex(index)) { _redraw = true; }
		}
	}
	InteractiveSurface::keyboardPress(action, state);
}


}
