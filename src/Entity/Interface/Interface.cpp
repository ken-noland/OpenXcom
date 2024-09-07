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
#include "Interface.h"



#include "Window.h"
#include "Button.h"

#include "../Engine/ECS.h"
#include "../Engine/Surface.h"
#include "../Engine/Palette.h"

#include "../Engine/Tickable.h"
#include "../Engine/Drawable.h"
#include "../Engine/Hierarchical.h"
#include "../Interface/Text.h"

#include "../../Engine/Timer.h"
#include "../../Mod/Mod.h"

namespace OpenXcom
{
InterfaceFactory::InterfaceFactory(ECS& ecs, Mod* mod)
	: _ecs(ecs), _mod(mod), _surfaceFactory(ecs.getFactory<SurfaceFactory>())
{
}

InterfaceFactory::~InterfaceFactory()
{
}

void InterfaceFactory::setMod(Mod* mod)
{
	_mod = mod;
}

Element InterfaceFactory::getElementFromRule(const std::string& ruleCategory, const std::string& ruleID, int x, int y, int width, int height, entt::handle parent)
{
	Element ret;
	ret.w = width;
	ret.h = height;
	ret.x = x;
	ret.y = y;
	ret.color = 0;
	ret.color2 = 0;
	ret.border = 0;
	ret.TFTDMode = false;

	// get the mod rule interface
	RuleInterface* ruleInterface = nullptr;

	if (_mod)
	{
		ruleInterface = _mod->getInterface(ruleID);
	}

	// get the element from the mod rule interface
	if (ruleInterface)
	{
		Element* element = ruleInterface->getElement(ruleCategory);
		if (element)
		{
			bool isParentValid = _ecs.getRegistry().raw().valid(parent);
			if (isParentValid && element->w != INT_MAX && element->h != INT_MAX)
			{
				ret.w = element->w;
				ret.h = element->h;
			}

			if (isParentValid && element->x != INT_MAX && element->y != INT_MAX)
			{
				ScreenRectComponent& parentRect = parent.get<ScreenRectComponent>();

				ret.x = parentRect.x + element->x;
				ret.y = parentRect.y + element->y;
			}

			ret.TFTDMode = element->TFTDMode;

			if (element->color != INT_MAX)
			{
				ret.color = element->color;
			}
			if (element->color2 != INT_MAX)
			{
				ret.color2 = element->color2;
			}
			if (element->border != INT_MAX)
			{
				ret.border = element->border;
			}
		}
	}

	return ret;
}

entt::handle InterfaceFactory::createArrowButton(const std::string& name, ArrowShape shape, int width, int height, int x, int y)
{
	return entt::handle(_ecs.getRegistry().raw(), entt::entity(entt::null));
}

entt::handle InterfaceFactory::createText(const CreateTextParams& params)
{
	HierarchySystem& hierarchySystem = _ecs.getSystem<HierarchySystem>();

	TextSystem& textSystem = _ecs.getSystem<TextSystem>();

	std::string ruleCategory = params.ruleCategory.empty() ? "text" : params.ruleCategory;

	PaletteHandle palette = params.palette;
	if (palette == PaletteHandle::Invalid)
	{
		palette = _ecs.getSystem<PaletteSystem>().getPaletteByID(params.ruleID);
	}

	// create the surface
	Element element = getElementFromRule(params.ruleCategory, params.ruleID, params.x, params.y, params.width, params.height, params.parent);
	entt::handle text = _surfaceFactory.createSurface(params.name, element.x, element.y, element.w, element.h, palette);

	
	if (params.parent.valid())
	{
		hierarchySystem.addChild(params.parent, text);
	}

	TextComponent& textComponent = text.emplace<TextComponent>(params.text);

	Font* bigFont = params.bigFont;
	Font* smallFont = params.smallFont;
	bool isSmall = params.isSmall;

	// get font from mod if it is not set
	if (bigFont == nullptr && _mod != nullptr)
	{
		bigFont = _mod->getFont("FONT_BIG");
	}
	if (smallFont == nullptr && _mod != nullptr)
	{
		smallFont = _mod->getFont("FONT_SMALL");
	}

	TextFontComponent& textFontComponent = text.emplace<TextFontComponent>(bigFont, smallFont, isSmall);
	TextAlignmentComponent& textAlignmentComponent = text.emplace<TextAlignmentComponent>(params.align, params.verticalAlign);

	textSystem.setColor(text, element.color);
	textSystem.setSecondaryColor(text, element.color2);

	DrawableComponent& drawableComponent = text.get<DrawableComponent>();
	drawableComponent.addDrawable(std::bind_front(&TextSystem::draw, textSystem, text));

	return text;
}

entt::handle InterfaceFactory::createTextButton(const CreateTextButtonParams& params)
{
	HierarchySystem& hierarchySystem = _ecs.getSystem<HierarchySystem>();

	std::string ruleCategory = params.ruleCategory.empty() ? "button" : params.ruleCategory;

	PaletteHandle palette = params.palette;
	if (palette == PaletteHandle::Invalid)
	{
		palette = _ecs.getSystem<PaletteSystem>().getPaletteByID(params.ruleID);
	}

	// create the surface
	Element element = getElementFromRule(params.ruleCategory, params.ruleID, params.x, params.y, params.width, params.height, params.parent);
	entt::handle button = _surfaceFactory.createSurface(params.name, element.x, element.y, element.w, element.h, palette);

	// Button Component
	ButtonComponent& buttonComponent = button.emplace<ButtonComponent>();

	HierarchyComponent& hierarchy = button.emplace<HierarchyComponent>();

	if (params.parent.valid())
	{
		hierarchySystem.addChild(params.parent, button);
	}

	ButtonSystem& buttonSystem = _ecs.getSystem<ButtonSystem>();
	DrawableComponent& drawableComponent = button.get<DrawableComponent>();
	drawableComponent.addDrawable(std::bind_front(&ButtonSystem::draw, buttonSystem, button));

	// create the text
	CreateTextParams textParams
	{
		.name = params.name + "_text",
		.text = params.text,
		.x = params.x,
		.y = params.y,
		.width = params.width,
		.height = params.height,
		.align = TextHAlign::ALIGN_CENTER,
		.verticalAlign = TextVAlign::ALIGN_MIDDLE,
		.wordWrap = false,
		.palette = params.palette,
		.ruleCategory = ruleCategory,
		.ruleID = params.ruleID,
		.parent = button
	};

	entt::handle text = createText(textParams);

	return button;
}

entt::handle InterfaceFactory::createWindow(const std::string& name, State* state, int width, int height, int x, int y, WindowPopup popup)
{
	CreateWindowParams params;

	params.name = name;
	params.width = width;
	params.height = height;
	params.x = x;
	params.y = y;
	params.popup = popup;

	params.ruleCategory = "window";
	params.ruleID = name;

	return createWindow(params);
}

entt::handle InterfaceFactory::createWindow(const CreateWindowParams& params)
{
	WindowSystem& windowSystem = _ecs.getSystem<WindowSystem>();

	std::string ruleCategory = params.ruleCategory.empty() ? "window" : params.ruleCategory;

	PaletteHandle palette = params.palette;
	if (palette == PaletteHandle::Invalid)
	{
		palette = _ecs.getSystem<PaletteSystem>().getPaletteByID(params.ruleID);
	}

	Element element = getElementFromRule(ruleCategory, params.ruleID, params.x, params.y, params.width, params.height, params.parent);
	entt::handle window = _surfaceFactory.createSurface(params.name, element.x, element.y, element.w, element.h, palette);

	WindowComponent& windowComponent = window.emplace<WindowComponent>(params.popup);

	// Hierarchical Component
	HierarchyComponent& hierarchy = window.emplace<HierarchyComponent>();
	
	// if we aren't popping up the window, we don't need a progress timer
	if (params.popup != WindowPopup::POPUP_NONE)
	{
		// Progress Timer Component
		ProgressTimerComponent::ProgressCallback updateProgress = std::bind_front(&WindowSystem::UpdateProgress, windowSystem);
		ProgressTimerComponent::CompleteCallback completeProgress = std::bind_front(&WindowSystem::CompleteProgress, windowSystem);
		ProgressTimerComponent& progressComponent = window.emplace<ProgressTimerComponent>(WindowSystem::POPUP_SPEED_MS, updateProgress, completeProgress);
	}
	else
	{
		windowSystem.CompleteProgress(window);
	}

	Surface* background = params.background;
	if (_mod)
	{
		std::string bgImageName = _mod->getInterface(params.ruleID)->getBackgroundImage();
		background = _mod->getSurface(bgImageName);
	}

	if(background)
	{
		// Background Component
		window.emplace<BackgroundComponent>(background);
	}

	// hmmm, maybe a separate color component?
	windowSystem.setColor(window, element.color);

	// I couldn't think of a better place to put this(maybe in the State startup?)
	windowSystem.playSound(window);

	return window;
}



}
