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
#include "TextButton.h"
#include "../Engine/Action.h"
#include "../Engine/State.h"

namespace OpenXcom
{

class ToggleTextButton :
	public TextButton
{
private:
	bool _isPressed;
	int _originalColor, _invertedColor;
	TextButton *_fakeGroup;

public:
	ToggleTextButton(int width, int height, int x, int y);
	~ToggleTextButton(void);

	void draw() override;
	void mousePress(Action *action, State *state) override;
	void setPressed(bool pressed);
	bool getPressed() const { return _isPressed; }
	void setColor(Uint8 color) override;
	void setInvertColor(Uint8 color);
};

}
