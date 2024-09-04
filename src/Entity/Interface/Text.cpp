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
#include "Text.h"
#include "../../Engine/Game.h"
#include "../../Mod/Mod.h"
#include "../../Engine/Font.h"

namespace OpenXcom
{

//TextComponent::TextComponent(const std::string& text, SurfaceComponent* surfaceComponent)
//	: _surfaceComponent(surfaceComponent),
//	_big(nullptr), _small(nullptr), _font(nullptr), _fontOrig(nullptr), _lang(nullptr),
//	_wrap(false), _invert(false), _contrast(false), _indent(false), _scroll(false), _ignoreSeparators(false),
//	_align(TextHAlign::ALIGN_LEFT), _valign(TextVAlign::ALIGN_TOP), _color(0), _color2(0), _scrollY(0)
//{
//	//KN TODO - Maybe use a font factory instead of getting these directly from the Mod?
//	_big = getGame()->getMod()->getFont("FONT_BIG");
//	_small = getGame()->getMod()->getFont("FONT_SMALL");
//
//	_lang = getGame()->getLanguage();
//
//	setSmall();
//
//	setText(text);
//}
//
//TextComponent::~TextComponent()
//{
//}
//
//void TextComponent::processText()
//{
//	if (_font == 0 || _lang == 0)
//	{
//		return;
//	}
//
//	Surface* surface = _surfaceComponent->getSurface();
//
//	_processedText = Unicode::convUtf8ToUtf32(_text);
//	_lineWidth.clear();
//	_lineHeight.clear();
//	_scrollY = 0;
//
//	int width = 0, word = 0;
//	size_t space = 0, textIndentation = 0;
//	bool start = true;
//	Font* font = _font;
//	UString& str = _processedText;
//
//	// Go through the text character by character
//	for (size_t c = 0; c <= str.size(); ++c)
//	{
//		// End of the line
//		if (c == str.size() || Unicode::isLinebreak(str[c]))
//		{
//			// Add line measurements for alignment later
//			_lineWidth.push_back(width);
//			_lineHeight.push_back(font->getCharSize('\n').h);
//			width = 0;
//			word = 0;
//			start = true;
//
//			if (c == str.size())
//				break;
//			else if (str[c] == Unicode::TOK_NL_SMALL)
//				font = _small;
//		}
//		// Keep track of spaces for wordwrapping
//		else if (Unicode::isSpace(str[c]) || (!_ignoreSeparators && Unicode::isSeparator(str[c])))
//		{
//			// Store existing indentation
//			if (c == textIndentation)
//			{
//				textIndentation++;
//			}
//			space = c;
//			width += font->getCharSize(str[c]).w;
//			word = 0;
//			start = false;
//		}
//		// Keep track of the width of the last line and word
//		else if (str[c] != Unicode::TOK_COLOR_FLIP)
//		{
//			int charWidth = font->getCharSize(str[c]).w;
//
//			width += charWidth;
//			word += charWidth;
//
//			// Wordwrap if the last word doesn't fit the line
//			if (_wrap && width >= surface->getWidth() && (!start || _lang->getTextWrapping() == WRAP_LETTERS))
//			{
//				size_t indentLocation = c;
//				if (_lang->getTextWrapping() == WRAP_WORDS || Unicode::isSpace(str[c]))
//				{
//					// Go back to the last space and put a linebreak there
//					width -= word;
//					indentLocation = space;
//					if (Unicode::isSpace(str[space]))
//					{
//						width -= font->getCharSize(str[space]).w;
//						str[space] = '\n';
//					}
//					else
//					{
//						str.insert(space + 1, 1, '\n');
//						indentLocation++;
//					}
//				}
//				else if (_lang->getTextWrapping() == WRAP_LETTERS)
//				{
//					// Go back to the last letter and put a linebreak there
//					str.insert(c, 1, '\n');
//					width -= charWidth;
//				}
//
//				// Keep initial indentation of text
//				if (textIndentation > 0)
//				{
//					str.insert(indentLocation + 1, textIndentation, '\t');
//					indentLocation += textIndentation;
//				}
//				// Indent due to word wrap.
//				if (_indent)
//				{
//					str.insert(indentLocation + 1, 1, '\t');
//					width += font->getCharSize('\t').w;
//				}
//
//				_lineWidth.push_back(width);
//				_lineHeight.push_back(font->getCharSize('\n').h);
//				if (_lang->getTextWrapping() == WRAP_WORDS)
//				{
//					width = word;
//				}
//				else if (_lang->getTextWrapping() == WRAP_LETTERS)
//				{
//					width = 0;
//				}
//				start = true;
//			}
//		}
//	}
//
//
//	_surfaceComponent->getSurface()->setRedraw(true);
//}
//
///**
// * Changes the text to use the big-size font.
// */
//void TextComponent::setBig()
//{
//	_font = _big;
//	_fontOrig = _big;
//	processText();
//}
//
///**
// * Changes the text to use the small-size font.
// */
//void TextComponent::setSmall()
//{
//	_font = _small;
//	_fontOrig = _small;
//	processText();
//}
//
///**
// * Returns the font currently used by the text.
// * @return Pointer to font.
// */
//Font* TextComponent::getFont() const
//{
//	return _font;
//}
//
///**
// * Changes the string displayed on screen.
// * @param text Text string.
// */
//void TextComponent::setText(const std::string& text)
//{
//	_text = text;
//	_font = _fontOrig;
//	processText();
//	Surface* surface = _surfaceComponent->getSurface();
//
//	// If big text won't fit the space, try small text
//	if (!_text.empty())
//	{
//		if (_font == _big && (getTextWidth() > surface->getWidth() || getTextHeight() > surface->getHeight()) && _text[_text.size() - 1] != '.')
//		{
//			_font = _small;
//			processText();
//		}
//	}
//}
//
///**
// * Returns the string displayed on screen.
// * @return Text string.
// */
//std::string TextComponent::getText() const
//{
//	return _text;
//}
//
///**
// * Enables/disables text wordwrapping. When enabled, lines of
// * text are automatically split to ensure they stay within the
// * drawing area, otherwise they simply go off the edge.
// * @param wrap Wordwrapping setting.
// * @param indent Indent wrapped text.
// * @param ignoreSeparators Handle separators as spaces (false) or as normal text (true)?
// */
//void TextComponent::setWordWrap(bool wrap, bool indent, bool ignoreSeparators)
//{
//	if (wrap != _wrap || indent != _indent || ignoreSeparators != _ignoreSeparators)
//	{
//		_wrap = wrap;
//		_indent = indent;
//		_ignoreSeparators = ignoreSeparators;
//		processText();
//	}
//}
//
///**
// * Enables/disables color inverting. Mostly used to make
// * button text look pressed along with the button.
// * @param invert Invert setting.
// */
//void TextComponent::setInvert(bool invert)
//{
//	_invert = invert;
//	_surfaceComponent->getSurface()->setRedraw(true);
//}
//
///**
// * Enables/disables high contrast color. Mostly used for
// * Battlescape UI.
// * @param contrast High contrast setting.
// */
//void TextComponent::setHighContrast(bool contrast)
//{
//	_contrast = contrast;
//	_surfaceComponent->getSurface()->setRedraw(true);
//}
//
///**
// * Changes the way the text is aligned horizontally
// * relative to the drawing area.
// * @param align Horizontal alignment.
// */
//void TextComponent::setAlign(TextHAlign align)
//{
//	_align = align;
//	_surfaceComponent->getSurface()->setRedraw(true);
//}
//
///**
// * Returns the way the text is aligned horizontally
// * relative to the drawing area.
// * @return Horizontal alignment.
// */
//TextHAlign TextComponent::getAlign() const
//{
//	return TextHAlign();
//}
//
///**
// * Changes the way the text is aligned vertically
// * relative to the drawing area.
// * @param valign Vertical alignment.
// */
//void TextComponent::setVerticalAlign(TextVAlign valign)
//{
//	_valign = valign;
//	_surfaceComponent->getSurface()->setRedraw(true);
//}
//
///**
// * Returns the way the text is aligned vertically
// * relative to the drawing area.
// * @return Horizontal alignment.
// */
//TextVAlign TextComponent::getVerticalAlign() const
//{
//	return _valign;
//}
//
///**
// * Changes the color used to render the text. Unlike regular graphics,
// * fonts are greyscale so they need to be assigned a specific position
// * in the palette to be displayed.
// * @param color Color value.
// */
//void TextComponent::setColor(Uint8 color)
//{
//	_color = color;
//	_color2 = color;
//	_surfaceComponent->getSurface()->setRedraw(true);
//}
//
///**
// * Returns the color used to render the text.
// * @return Color value.
// */
//Uint8 TextComponent::getColor() const
//{
//	return _color;
//}
//
///**
// * Changes the secondary color used to render the text. The text
// * switches between the primary and secondary color whenever there's
// * a 0x01 in the string.
// * @param color Color value.
// */
//void TextComponent::setSecondaryColor(Uint8 color)
//{
//	_color2 = color;
//	_surfaceComponent->getSurface()->setRedraw(true);
//}
//
///**
// * Returns the secondary color used to render the text.
// * @return Color value.
// */
//Uint8 TextComponent::getSecondaryColor() const
//{
//	return _color2;
//}
//
//int TextComponent::getNumLines() const
//{
//	return _wrap ? (int)_lineHeight.size() : 1;
//}
//
///**
// * Returns the rendered text's height. Useful to check if wordwrap applies.
// * @param line Line to get the height, or -1 to get whole text height.
// * @return Height in pixels.
// */
//int TextComponent::getTextHeight(int line) const
//{
//	if (line == -1)
//	{
//		int height = 0;
//		for (int lh : _lineHeight)
//		{
//			height += lh;
//		}
//		return height;
//	}
//	else
//	{
//		return _lineHeight[line];
//	}
//}
//
///**
// * Returns the rendered text's width.
// * @param line Line to get the width, or -1 to get whole text width.
// * @return Width in pixels.
// */
//int TextComponent::getTextWidth(int line) const
//{
//	if (line == -1)
//	{
//		int width = 0;
//		for (int lw : _lineWidth)
//		{
//			if (lw > width)
//			{
//				width = lw;
//			}
//		}
//		return width;
//	}
//	else
//	{
//		return _lineWidth[line];
//	}
//}
//
//void TextComponent::setScrollable(bool scroll)
//{
//	_scroll = scroll;
//	_surfaceComponent->getSurface()->setRedraw(true);
//}
//
//void TextComponent::draw()
//{
//	//Surface::draw();
//	if (_text.empty() || _font == 0)
//	{
//		return;
//	}
//
//	//// Show text borders for debugging
//	//if (Options::debugUi)
//	//{
//	//	SDL_Rect r;
//	//	r.w = getWidth();
//	//	r.h = getHeight();
//	//	r.x = 0;
//	//	r.y = 0;
//	//	this->drawRect(&r, 5);
//	//	r.w -= 2;
//	//	r.h -= 2;
//	//	r.x++;
//	//	r.y++;
//	//	this->drawRect(&r, 0);
//	//}
//
//	int x = 0, y = 0, line = 0, height = 0;
//	Font* font = _font;
//	int color = _color;
//	const UString& s = _processedText;
//
//	height = getTextHeight();
//
//	if (_scroll && (getHeight() - height < 0))
//	{
//		y = _scrollY;
//	}
//	else
//	{
//		switch (_valign)
//		{
//		case TextVAlign::ALIGN_TOP:
//			y = 0;
//			break;
//		case TextVAlign::ALIGN_MIDDLE:
//			y = (int)ceil((getHeight() - height) / 2.0);
//			break;
//		case TextVAlign::ALIGN_BOTTOM:
//			y = getHeight() - height;
//			break;
//		}
//	}
//
//	x = getLineX(line);
//
//	// Set up text color
//	int mul = 1;
//	if (_contrast)
//	{
//		mul = 3;
//	}
//
//	// Set up text direction
//	int dir = 1;
//	if (_lang->getTextDirection() == DIRECTION_RTL)
//	{
//		dir = -1;
//	}
//
//	// Invert text by inverting the font palette on index 3 (font palettes use indices 1-5)
//	int mid = _invert ? 3 : 0;
//
//	// Draw each letter one by one
//	for (UString::const_iterator c = s.begin(); c != s.end(); ++c)
//	{
//		if (Unicode::isSpace(*c) || *c == '\t')
//		{
//			x += dir * font->getCharSize(*c).w;
//		}
//		else if (Unicode::isLinebreak(*c))
//		{
//			line++;
//			y += font->getCharSize(*c).h;
//			x = getLineX(line);
//			if (*c == Unicode::TOK_NL_SMALL)
//			{
//				font = _small;
//			}
//		}
//		else if (*c == Unicode::TOK_COLOR_FLIP)
//		{
//			color = (color == _color ? _color2 : _color);
//		}
//		else
//		{
//			if (dir < 0)
//				x += dir * font->getCharSize(*c).w;
//			SurfaceCrop chr = font->getChar(*c);
//			chr.setX(x);
//			chr.setY(y);
//			ShaderDraw<PaletteShift>(ShaderSurface(this, 0, 0), ShaderCrop(chr), ShaderScalar(color), ShaderScalar(mul), ShaderScalar(mid));
//			if (dir > 0)
//				x += dir * font->getCharSize(*c).w;
//		}
//	}
//}

}
