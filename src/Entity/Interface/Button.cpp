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
#include "Button.h"
#include "Text.h"
#include "../../Engine/Game.h"
#include "../../Engine/Screen.h"

#include "Interface.h"
#include "../Engine/Palette.h"
#include "../Engine/Hierarchical.h"
#include "../Engine/Drawable.h"

#include "../Common/RTTR.h"


namespace OpenXcom
{

SIMPLERTTR
{
	/// RTTR Registration for ButtonComponent
	SimpleRTTR::registration().type<ButtonComponent>()
		.meta(GetComponentFuncName, &GetComponentRawPointer<ButtonComponent>)
		.property(&ButtonComponent::_color, "color")
		.property(&ButtonComponent::_contrast, "contrast")
		.property(&ButtonComponent::_geoscapeButton, "geoscapeButton");
}

void ButtonSystem::setColor(entt::handle entity, uint8_t color)
{
	ButtonComponent& buttonComponent = entity.get<ButtonComponent>();
	buttonComponent._color = color;
}

uint8_t ButtonSystem::getColor(entt::handle entity) const
{
	const ButtonComponent& buttonComponent = entity.get<ButtonComponent>();
	return buttonComponent._color;
}

void ButtonSystem::draw(entt::handle buttonEntity)
{
	//ButtonComponent& buttonComponent = buttonEntity.get<ButtonComponent>();
	//SurfaceComponent& surfaceComponent = buttonEntity.get<SurfaceComponent>();
	//ScreenRectComponent& screenRectComponent = buttonEntity.get<ScreenRectComponent>();

	//Surface* buttonSurface = surfaceComponent.getSurface();

	//buttonSurface->draw();

	//SDL_Rect square;

	//int mul = 1;
	//if (buttonComponent._contrast) {
	//	mul = 2;
	//}

	//int color = buttonComponent._color + 1 * mul;

	//square.x = 0;
	//square.y = 0;
	//square.w = screenRectComponent.width;
	//square.h = screenRectComponent.height;

	//for (int i = 0; i < 5; ++i) {
	//	buttonSurface->drawRect(&square, color);

	//	if (i % 2 == 0) {
	//		square.x++;
	//		square.y++;
	//	}
	//	square.w--;
	//	square.h--;

	//	switch (i) {
	//	case 0:
	//		color = buttonComponent._color + 5 * mul;
	//		buttonSurface->setPixel(square.w, 0, color);
	//		break;
	//	case 1:
	//		color = buttonComponent._color + 2 * mul;
	//		break;
	//	case 2:
	//		color = buttonComponent._color + 4 * mul;
	//		buttonSurface->setPixel(square.w + 1, 1, color);
	//		break;
	//	case 3:
	//		color = buttonComponent._color + 3 * mul;
	//		break;
	//	case 4:
	//		if (buttonComponent._geoscapeButton) {
	//			buttonSurface->setPixel(0, 0, buttonComponent._color);
	//			buttonSurface->setPixel(1, 1, buttonComponent._color);
	//		}
	//		break;
	//	}
	//}

	//SDL_Rect target{};
	//target.x = screenRectComponent.x;
	//target.y = screenRectComponent.y;
	//SDL_BlitSurface(buttonSurface->getSDLSurface(), nullptr, getGame()->getScreen()->getSurface(), &target);

	//// Now render the button contents
	//HierarchySystem& hierarchySystem = getSystem<HierarchySystem>();
	//hierarchySystem.visit(buttonEntity, [](entt::handle child) {
	//	DrawableSystem& drawableSystem = getSystem<DrawableSystem>();
	//	drawableSystem.draw(child);
	//});
}

void ButtonSystem::handle(entt::handle entity, Action* action)
{
}

//
//TextButtonComponent::TextButtonComponent(const std::string& text, SurfaceComponent& surfaceComponent)
//	: _surfaceComponent(surfaceComponent), _color(0), _contrast(false), _geoscapeButton(false), _comboBox(nullptr)
//{
//	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();
//
//	Surface* surface = surfaceComponent.getSurface();
//	int width = surface->getWidth();
//	int height = surface->getHeight();
//
//	//InterfaceFactory::CreateTextParams params{"text", text, 0, 0, width, height, surface->getPalette(), 0, 256 };
//	//_text = factory.createText(params);
//
//	//TextComponent& textComponent = _text.get<TextComponent>();
//
//	//textComponent.setAlign(TextHAlign::ALIGN_CENTER);
//	//textComponent.setVerticalAlign(TextVAlign::ALIGN_MIDDLE);
//	//textComponent.setWordWrap(true);
//}
//
//TextButtonComponent::~TextButtonComponent()
//{
//	//_text.destroy();
//}
//
///**
// * Changes the color for the button and text.
// * @param color Color value.
// */
//void TextButtonComponent::setColor(uint8_t color)
//{
//	//_color = color;
//
//	//TextComponent& textComponent = _text.get<TextComponent>();
//	//textComponent.setColor(color);
//
//	//_surfaceComponent.getSurface()->setRedraw(true);
//}
//
///**
// * Returns the color for the button and text.
// * @return Color value.
// */
//uint8_t TextButtonComponent::getColor() const
//{
//	return _color;
//}
//
///**
// * Changes the color for the text only.
// * @param color Color value.
// */
//void TextButtonComponent::setTextColor(uint8_t color)
//{
//	//TextComponent& textComponent = _text.get<TextComponent>();
//	//textComponent.setColor(color);
//
//	//_surfaceComponent.getSurface()->setRedraw(true);
//}
//
///**
// * Changes the text to use the big-size font.
// */
//void TextButtonComponent::setBig()
//{
//	//TextComponent& textComponent = _text.get<TextComponent>();
//	//textComponent.setBig();
//
//	//_surfaceComponent.getSurface()->setRedraw(true);
//}
//
///**
// * Changes the text to use the small-size font.
// */
//void TextButtonComponent::setSmall()
//{
//	//TextComponent& textComponent = _text.get<TextComponent>();
//	//textComponent.setSmall();
//
//	//_surfaceComponent.getSurface()->setRedraw(true);
//}
//
///**
// * Returns the font currently used by the text.
// * @return Pointer to font.
// */
//Font* TextButtonComponent::getFont() const
//{
//	//TextComponent& textComponent = _text.get<TextComponent>();	
//	//return textComponent.getFont();
//}
//
///**
// * Enables/disables high contrast color. Mostly used for
// * Battlescape UI.
// * @param contrast High contrast setting.
// */
//void TextButtonComponent::setHighContrast(bool contrast)
//{
//	//_contrast = contrast;
//
//	//TextComponent& textComponent = _text.get<TextComponent>();
//	//textComponent.setHighContrast(contrast);
//
//	//_surfaceComponent.getSurface()->setRedraw(true);
//}
//
///**
// * Changes the text of the button label.
// * @param text Text string.
// */
//void TextButtonComponent::setText(const std::string& text)
//{
//	//TextComponent& textComponent = _text.get<TextComponent>();
//	//textComponent.setText(text);
//
//	//_surfaceComponent.getSurface()->setRedraw(true);
//}
//
///**
// * Returns the text of the button label.
// * @return Text string.
// */
//std::string TextButtonComponent::getText() const
//{
//	//TextComponent& textComponent = _text.get<TextComponent>();
//	//return textComponent.getText();
//}
//
///**
// * Changes the button group this button belongs to.
// * @param group Pointer to the pressed button pointer in the group.
// * Null makes it a regular button.
// */
////void TextButtonComponent::setGroup(TextButton** group)
////{
////	_group = group;
////	_redraw = true;
////}
//
////void TextButtonComponent::setPalette(PaletteComponent& )
////{
////	_surfaceComponent.getSurface()->setPalette(colors, firstcolor, ncolors);
//
////	_text->setPalette(colors, firstcolor, ncolors);
////}
//
//
///**
// * Draws the labeled button.
// * The colors are inverted if the button is pressed.
// */
//void TextButtonComponent::draw()
//{
//	//Surface::draw();
//	Surface* surface = _surfaceComponent.getSurface();
//	//surface->draw();
//
//	SDL_Rect square;
//
//	int mul = 1;
//	if (_contrast)
//	{
//		mul = 2;
//	}
//
//	int color = _color + 1 * mul;
//
//	square.x = 0;
//	square.y = 0;
//	square.w = surface->getWidth();
//	square.h = surface->getHeight();
//
//	for (int i = 0; i < 5; ++i)
//	{
//		surface->drawRect(&square, color);
//
//		if (i % 2 == 0)
//		{
//			square.x++;
//			square.y++;
//		}
//		square.w--;
//		square.h--;
//
//		switch (i)
//		{
//		case 0:
//			color = _color + 5 * mul;
//			surface->setPixel(square.w, 0, color);
//			break;
//		case 1:
//			color = _color + 2 * mul;
//			break;
//		case 2:
//			color = _color + 4 * mul;
//			surface->setPixel(square.w + 1, 1, color);
//			break;
//		case 3:
//			color = _color + 3 * mul;
//			break;
//		case 4:
//			if (_geoscapeButton)
//			{
//				surface->setPixel(0, 0, _color);
//				surface->setPixel(1, 1, _color);
//			}
//			break;
//		}
//	}
//
//	//bool press;
//	//if (_group == 0)
//	//	press = isButtonPressed();
//	//else
//	//	press = (*_group == this);
//
//	//if (press)
//	//{
//	//	if (_geoscapeButton)
//	//	{
//	//		this->invert(_color + 2 * mul);
//	//	}
//	//	else
//	//	{
//	//		this->invert(_color + 3 * mul);
//	//	}
//	//}
//	//_text->setInvert(press);
////	_text.setColor(1);
//
//	
//	//_text->setPalette(surface->getPalette());
//
//	//_text->draw();
//	//_text->blit(surface->getSurface());
//}
//
///**
// * Sets the button as the pressed button if it's part of a group.
// * @param action Pointer to an action.
// * @param state State that the action handlers belong to.
// */
////void TextButtonComponent::mousePress(Action* action, State* state)
////{
//	//if (action->getDetails()->button.button == SDL_BUTTON_LEFT && _group != 0)
//	//{
//	//	TextButton* old = *_group;
//	//	*_group = this;
//	//	if (old != 0)
//	//		old->draw();
//	//	draw();
//	//}
//
//	//if (isButtonHandled(action->getDetails()->button.button))
//	//{
//	//	if (soundPress != 0 && _group == 0 &&
//	//		action->getDetails()->button.button != SDL_BUTTON_WHEELUP && action->getDetails()->button.button != SDL_BUTTON_WHEELDOWN)
//	//	{
//	//		soundPress->play(Mix_GroupAvailable(0));
//	//	}
//
//	//	if (_comboBox)
//	//	{
//	//		_comboBox->toggle(false, false);
//	//	}
//
//	//	draw();
//	//	//_redraw = true;
//	//}
//	//InteractiveSurface::mousePress(action, state);
////}
//
///**
// * Sets the button as the released button.
// * @param action Pointer to an action.
// * @param state State that the action handlers belong to.
// */
////void TextButtonComponent::mouseRelease(Action* action, State* state)
////{
//	//if (isButtonHandled(action->getDetails()->button.button))
//	//{
//	//	draw();
//	//	//_redraw = true;
//	//}
//	//InteractiveSurface::mouseRelease(action, state);
////}
//
///**
// * Hooks up the button to work as part of an existing combobox,
// * toggling its state when it's pressed.
// * @param comboBox Pointer to ComboBox.
// */
//void TextButtonComponent::setComboBox(ComboBox* comboBox)
//{
//	_comboBox = comboBox;
//
//	//SurfaceComponent& textSurfaceComponent = _text.get<SurfaceComponent>();
//	//Surface* textSurface = textSurfaceComponent.getSurface();
//	//if (_comboBox)
//	//{
//	//	textSurface->setX(-6);
//	//}
//	//else
//	//{
//	//	textSurface->setX(0);
//	//}
//}
//
//void TextButtonComponent::setGeoscapeButton(bool geo)
//{
//	_geoscapeButton = geo;
//}
//


} // namespace OpenXcom
