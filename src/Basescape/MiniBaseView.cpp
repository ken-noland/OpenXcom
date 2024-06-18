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
#include "../Engine/Action.h"
#include "../Engine/Registry.h"
#include "../Engine/SurfaceSet.h"
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
MiniBaseView::MiniBaseView(int width, int height, int x, int y)
	: InteractiveSurface(width, height, x, y)
{
	getRegistry().raw().on_construct<Base>().connect<&MiniBaseView::onBaseSetChange>(this);
	getRegistry().raw().on_destroy<Base>().connect<&MiniBaseView::onBaseSetChange>(this);
}

MiniBaseView::~MiniBaseView()
{
	getRegistry().raw().on_construct<Base>().disconnect<&MiniBaseView::onBaseSetChange>(this);
	getRegistry().raw().on_destroy<Base>().disconnect<&MiniBaseView::onBaseSetChange>(this);
}

/**
 * Changes the base that is currently selected on
 * the mini base view.
 * @param base ID of base.
 */
void MiniBaseView::setSelectedBaseIndex(int selectedBaseIndex)
{
	_selectedBaseIndex = selectedBaseIndex;
	_redraw = true;
}

/**
 * Changes the set of bases that are currently visible on
 * the mini base view (if more than MAX_VISIBLE_selectedBaseIndex)
 * to show one more up
 */
bool MiniBaseView::incVisibleBasesIndex()
{
	if (_visibleBasesIndexOffset < (_baseIds.size() - MAX_VISIBLE_BASES))
	{
		_visibleBasesIndexOffset++;
		_redraw = true;
		return true;
	}
	return false;
}

/**
 * Changes the set of bases that are currently visible on
 * the mini base view (if more than MAX_VISIBLE_selectedBaseIndex)
 * to show one more down
 */
bool MiniBaseView::decVisibleBasesIndex()
{
	if (_visibleBasesIndexOffset > 0)
	{
		_visibleBasesIndexOffset--;
		_redraw = true;
		return true;
	}
	return false;
}

/**
 * Draws the view of all the bases with facilities
 * in varying colors.
 */
void MiniBaseView::draw()
{
	Surface::draw();

	auto bases = getRegistry().list<const Base>() | std::views::drop(_visibleBasesIndexOffset);
	auto baseIterator = bases.begin();

	for (Sint16 index = 0; index < static_cast<Sint16>(MAX_VISIBLE_BASES); ++index)
	{
		// Draw base squares
		if (index + _visibleBasesIndexOffset == _selectedBaseIndex)
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
		if (baseIterator != bases.end())
		{
			SDL_Rect r;
			lock();
			for (const BaseFacility* fac : (*baseIterator).getFacilities())
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
		}
	}
}

/**
 * Selects the base the mouse is over.
 * @param action Pointer to an action.
 * @param state State that the action handlers belong to.
 */
void MiniBaseView::mouseOver(Action *action, State *state)
{
	_hoveredBaseIndex = static_cast<int>(floor(action->getRelativeXMouse() / ((MINI_SIZE + 2) * action->getXScale())));
	InteractiveSurface::mouseOver(action, state);
}

}
