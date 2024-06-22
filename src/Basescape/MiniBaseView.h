#pragma once
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
#include <entt/entt.hpp>
#include "../Engine/InteractiveSurface.h"

namespace OpenXcom
{

class Base;
class BasescapeSystem;
class SurfaceSet;

/**
 * Mini view of a base.
 * Takes all the bases and displays their layout
 * and allows players to swap between them.
 */
class MiniBaseView : public InteractiveSurface
{
private:
	static const int MINI_SIZE = 14;

	BasescapeSystem& _basescapeSystem;
	size_t _unsubscribeId;

	std::array<SDLKey, 8> _baseKeys;

	SurfaceSet* _texture = nullptr;

	Uint8 _red = 0, _green = 0, _blue = 0;

public:
	/// Creates a new mini base view at the specified position and size.
	MiniBaseView(BasescapeSystem& basescapeSystem, int width, int height, int x = 0, int y = 0);
	/// Removes MiniBaseView subscriptions.
	~MiniBaseView();
	/// Invoked whenever the set of bases is changed (created/destroyed). Invokes a redraw
	void onBaseSetChange(entt::registry&, entt::entity) { _redraw = true; }
	/// Sets the texture for the mini base view.
	void setTexture(SurfaceSet* texture) { _texture = texture; }

	void mouseClick(Action* action, State* state) override;

	void keyboardPress(Action* action, State* state) override;

	void draw() override;

	void setColor(Uint8 color) override { _green = color; }
	void setSecondaryColor(Uint8 color) override { _red = color; }
	void setBorderColor(Uint8 color) override { _blue = color; }
};

}
