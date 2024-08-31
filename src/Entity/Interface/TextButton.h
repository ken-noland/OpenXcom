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

class Text;
class ComboBox;
class SurfaceComponent;
class TickableComponent;
class DrawableComponent;
class PaletteComponent;


class TextButtonComponent
{
private:
	SurfaceComponent& _surfaceComponent;

	Uint8 _color;
	Text* _text;

	//TextButton** _group;
	bool _contrast, _geoscapeButton;
	ComboBox* _comboBox;
	// for use by RuleInterface
//	void setSecondaryColor(Uint8 color) override { setTextColor(color); }

protected:
//	bool isButtonHandled(Uint8 button = 0) override;

public:
	TextButtonComponent(const std::string& text, SurfaceComponent& surfaceComponent);
	~TextButtonComponent();

	/// Sets the text button's color.
	void setColor(Uint8 color); //override
	/// Gets the text button's color.
	Uint8 getColor() const;
	/// Sets the text button's text color.
	void setTextColor(Uint8 color);
	/// Sets the text size to big.
	void setBig();
	/// Sets the text size to small.
	void setSmall();
	/// Gets the text button's current font.
	Font* getFont() const;

	/// Sets the text button's high contrast color setting.
	void setHighContrast(bool contrast); // override
	/// Sets the text button's text.
	void setText(const std::string& text);
	/// Gets the text button's text.
	std::string getText() const;

	/// Sets the text button's group.
	//void setGroup(TextButton** group);

	/// Draws the text button.
	void draw();	//override
	/// Special handling for mouse presses.
	//void mousePress(Action* action, State* state); //override
	///// Special handling for mouse releases.
	//void mouseRelease(Action* action, State* state); //override
	/// Attaches this button to a combobox.
	void setComboBox(ComboBox* comboBox);

	/// Sets the button as a geoscape button.
	void setGeoscapeButton(bool geo);
};

} // namespace OpenXcom
