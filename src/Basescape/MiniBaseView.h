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
#include <vector>
#include <entt/entt.hpp>
#include "../Engine/InteractiveSurface.h"

namespace OpenXcom
{

class Base;
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

	std::vector<entt::entity> _baseIds;
	SurfaceSet* _texture = nullptr;
	// the currently selected index
	int _selectedBaseIndex = -1;
	// the index the mouse if over
	int _hoveredBaseIndex = -1;
	// the amount of space the displayed index set is offset from the global set
	int _visibleBasesIndexOffset = 0;
	Uint8 _red = 0, _green = 0, _blue = 0;

public:
	static const size_t MAX_VISIBLE_BASES = 8;  
	/// Creates a new mini base view at the specified position and size.
	MiniBaseView(int width, int height, int x = 0, int y = 0);
	/// Removes MiniBaseView subscriptions.
	~MiniBaseView();
	/// Invoked whenever the set of bases is changed (created/destroyed). Invokes a redraw
	void onBaseSetChange(entt::registry&, entt::entity) { _redraw = true; }
	/// Sets the texture for the mini base view.
	void setTexture(SurfaceSet* texture) { _texture = texture; }
	/// Gets the base the mouse is over.
	int getHoveredBaseIndex() const { return _hoveredBaseIndex; }
	/// Sets the selected base for the mini base view.
	void setSelectedBaseIndex(int selectedBaseIndex);
	/// Increment index of visible bases for the mini base view (if possible).
	bool incVisibleBasesIndex();	
	/// Decrement index of visible bases for the mini base view (if possible).
	bool decVisibleBasesIndex();	
	/// Gets the index of visible bases for the mini base view.
	int getVisibleBasesIndexOffset() const { return _visibleBasesIndexOffset; }
	/// Sets the index of visible bases for the mini base view.
	void setVisibleBasesIndexOffset(int visibleBaseIndexOffset) { _visibleBasesIndexOffset = visibleBaseIndexOffset; }
	/// Draws the mini base view.
	void draw() override;
	/// Special handling for mouse hovers.
	void mouseOver(Action *action, State *state) override;
	void setColor(Uint8 color) override { _green = color; }
	void setSecondaryColor(Uint8 color) override { _red = color; }
	void setBorderColor(Uint8 color) override { _blue = color; }
};

}
