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
#include "ArrowButton.h"
#include "Text.h"
#include "TextButton.h"

#include "../Engine/ECS.h"
#include "../Engine/Surface.h"

#include "../Engine/Tickable.h"
#include "../Engine/Drawable.h"
#include "../Engine/Palette.h"
#include "../Engine/Hierarchical.h"

#include "../../Engine/Game.h"
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
		ruleInterface = _mod->getInterface(ruleCategory);
	}

	// get the element from the mod rule interface
	if (ruleInterface)
	{
		Element* element = ruleInterface->getElement(ruleID);
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
				SurfaceComponent& parentSurface = parent.get<SurfaceComponent>();

				ret.x = parentSurface.getX() + element->x;
				ret.y = parentSurface.getY() + element->y;
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
	Element element = getElementFromRule(params.ruleCategory, params.ruleID, params.x, params.y, params.width, params.height, params.parent);
	entt::handle entity = _surfaceFactory.createSurface(params.name, element.x, element.y, element.w, element.h, params.palette, params.firstColor, params.nColors);

	//SurfaceComponent& surfaceComponent = entity.get<SurfaceComponent>();
	//TextComponent& textComponent = entity.emplace<TextComponent>(params.text, &surfaceComponent);

	//textComponent.setColor(element.color);
	//textComponent.setSecondaryColor(element.color2);

	//DrawableComponent& drawableComponent = _registry.get<DrawableComponent>(entity);
	//drawableComponent.addDrawable(std::bind(&TextComponent::draw, &entity.get<TextComponent>()));



	return entity;
}

entt::handle InterfaceFactory::createTextButton(const CreateTextButtonParams& params)
{
	Element element = getElementFromRule(params.ruleCategory, params.ruleID, params.x, params.y, params.width, params.height, params.parent);

	//create the button
	entt::handle button = _surfaceFactory.createSurface(params.name, element.x, element.y, element.w, element.h, params.palette, params.firstColor, params.nColors);


	if (params.parent.valid())
	{
		HierarchySystem& hierarchySystem = _ecs.getSystem<HierarchySystem>();
		hierarchySystem.addChild(params.parent, button);
	}

	//SurfaceComponent& surfaceComponent = button.get<SurfaceComponent>();
	//TextButtonComponent& textButtonComponent = button.emplace<TextButtonComponent>(params.text, surfaceComponent);
	//textButtonComponent.setColor(element.color);

	//DrawableComponent& drawableComponent = button.get<DrawableComponent>();
	//drawableComponent.addDrawable(std::bind(&TextButtonComponent::draw, &textButtonComponent));



	//entt::handle text = _surfaceFactory.createText(params.name, element.x, element.y, element.w, element.h, params.palette, params.firstColor, params.nColors);

	//// add text to the button
	//HierarchyComponent& hierarchy = button.emplace<HierarchyComponent>();
	//hierarchy.addChild(text);

	return button;
}

entt::handle InterfaceFactory::createWindow(const std::string& name, State* state, int width, int height, int x, int y, WindowPopup popup)
{
	CreateWindowParams params;

	params.name = name;
	params.state = state;
	params.width = width;
	params.height = height;
	params.x = x;
	params.y = y;
	params.popup = popup;

	params.palette = state->getPalette();
	params.firstColor = 0;
	params.nColors = 256;

	params.ruleCategory = "window";
	params.ruleID = name;

	return createWindow(params);
}

entt::handle InterfaceFactory::createWindow(const CreateWindowParams& params)
{
	WindowSystem& windowSystem = _ecs.getSystem<WindowSystem>();

	Element element = getElementFromRule(params.ruleCategory, params.ruleID, params.x, params.y, params.width, params.height, params.parent);
	entt::handle window = _surfaceFactory.createSurface(params.name, element.x, element.y, element.w, element.h, params.palette, params.firstColor, params.nColors);

	WindowComponent& windowComponent = window.emplace<WindowComponent>(params.popup);

//KN NOTE: I'm going to remove tickable and drawable component in the near future, but I'm just getting the timer set up and I don't have time to deal with the added mess
TickableComponent& tickableComponent = window.emplace<TickableComponent>();
//tickableComponent.addTickable(std::bind(&WindowComponent::tick, &windowComponent));


	// Hierarchical Component
	HierarchyComponent& hierarchy = window.emplace<HierarchyComponent>();
	
	// Progress Timer Component

	// if we aren't popping up the window, we don't need a progress timer
	if (params.popup != WindowPopup::POPUP_NONE)
	{
		ProgressTimerComponent::ProgressCallback updateProgress = std::bind_front(&WindowSystem::UpdateProgress, windowSystem);
		ProgressTimerComponent::CompleteCallback completeProgress = std::bind_front(&WindowSystem::CompleteProgress, windowSystem);
		ProgressTimerComponent& progressComponent = window.emplace<ProgressTimerComponent>(WindowSystem::POPUP_SPEED_MS, updateProgress, completeProgress);
	}
	else
	{
		windowSystem.CompleteProgress(window);
	}

	// hmmm, maybe a separate color component?
	windowSystem.setColor(window, element.color);
	windowSystem.playSound(window);

	return window;
}



}
