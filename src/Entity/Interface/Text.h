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
#include "../../Engine/Unicode.h"

//KN NOTE: temp - until I move the rest of the definitions here
#include "../../Interface/Text.h"

namespace OpenXcom
{


struct TextComponent
{
	Font *_big, *_small, *_font, *_fontOrig;
	Language* _lang;
	std::string _text;
	UString _processedText;
	std::vector<int> _lineWidth, _lineHeight;
	bool _wrap, _invert, _contrast, _indent, _scroll, _ignoreSeparators;
	TextHAlign _align;
	TextVAlign _valign;
	Uint8 _color, _color2;
	int _scrollY;
};


class TextSystem
{
private:

	/// Processes the contained text.
	void processText();
	/// Gets the X position of a text line.
	int getLineX(int line) const;

public:
	TextSystem();
	~TextSystem();

	/// Sets the text size to big.
	void setBig(entt::handle textHandle);
	/// Sets the text size to small.
	void setSmall(entt::handle textHandle);
	/// Gets the text's current font.
	Font* getFont(entt::handle textHandle) const;

	/// Sets the text's string.
	void setText(entt::handle textHandle, const std::string& text);
	/// Gets the text's string.
	std::string getText(entt::handle textHandle) const;

	/// Sets the text's wordwrap setting.
	void setWordWrap(entt::handle textHandle, bool wrap, bool indent = false, bool ignoreSeparators = false);
	/// Sets the text's color invert setting.
	void setInvert(entt::handle textHandle, bool invert);

	/// Sets the text's high contrast color setting.
	void setHighContrast(entt::handle textHandle, bool contrast); // override;

	/// Sets the text's horizontal alignment.
	void setAlign(entt::handle textHandle, TextHAlign align);
	/// Gets the text's horizontal alignment.
	TextHAlign getAlign(entt::handle textHandle) const;

	/// Sets the text's vertical alignment.
	void setVerticalAlign(entt::handle textHandle, TextVAlign valign);
	/// Gets the text's vertical alignment.
	TextVAlign getVerticalAlign(entt::handle textHandle) const;

	/// Sets the text's color.
	void setColor(entt::handle textHandle, Uint8 color); // override;
	/// Gets the text's color.
	Uint8 getColor(entt::handle textHandle) const;

	/// Sets the text's secondary color.
	void setSecondaryColor(entt::handle textHandle, Uint8 color);
	/// Gets the text's secondary color.
	Uint8 getSecondaryColor(entt::handle textHandle) const;

	/// Gets the number of lines in the (wrapped, if wrapping is enabled) text
	int getNumLines(entt::handle textHandle) const;
	/// Gets the rendered text's width.
	int getTextWidth(entt::handle textHandle, int line = -1) const;
	/// Gets the rendered text's height.
	int getTextHeight(entt::handle textHandle, int line = -1) const;

	/// Draws the text.
	void draw(entt::handle textHandle); // override;

	/// Sets the text's scrollable setting.
	void setScrollable(entt::handle textHandle, bool scroll);
	/// Special handling for mouse presses.
	//void mousePress(Action* action, State* state) override;
};

}
