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
#include "../Engine/Surface.h"



namespace OpenXcom
{

class Action;
class Text;
class ComboBox;

/**
 * Component for buttons.
 */
struct ButtonComponent
{
	Uint8 _color;

	bool _contrast, _geoscapeButton;
	ComboBox* _comboBox;
};

/**
 * System for handling buttons.
 */
class ButtonSystem
{
public:
	ButtonSystem() = default;
	~ButtonSystem() = default;

	/// Sets the text button's color.
	void setColor(entt::handle entity, Uint8 color);

	/// Gets the text button's color.
	Uint8 getColor(entt::handle entity) const;
		
	/// Draws the text button.
	void draw(entt::handle entity);
		
	/// Handles the input for the button.
	void handle(entt::handle entity, Action* action);
};

} // namespace OpenXcom
