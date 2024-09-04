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
#include <entt/entt.hpp>
#include "Window.h"

//temp includes
#include "../../Interface/ArrowButton.h"
#include "../../Interface/Bar.h"
#include "../../Interface/BattlescapeButton.h"
#include "../../Interface/ComboBox.h"
#include "../../Interface/Cursor.h"
#include "../../Interface/FpsCounter.h"
#include "../../Interface/Frame.h"
#include "../../Interface/ImageButton.h"
#include "../../Interface/NumberText.h"
#include "../../Interface/ProgressBar.h"
#include "../../Interface/ScrollBar.h"
#include "../../Interface/Slider.h"
#include "../../Interface/Text.h"
#include "../../Interface/TextButton.h"
#include "../../Interface/TextEdit.h"
#include "../../Interface/TextList.h"
#include "../../Interface/ToggleTextButton.h"

#include "../../Mod/RuleInterface.h"


namespace OpenXcom
{

class ECS;
class SurfaceFactory;
class Mod;

class InterfaceFactory
{
protected:
	ECS& _ecs;
	Mod* _mod;

	//cache the surface factory since we will be using it a lot
	SurfaceFactory& _surfaceFactory;

	Element getElementFromRule(const std::string& ruleCategory, const std::string& ruleID, int x, int y, int width, int height, entt::handle parent);

public:
	InterfaceFactory(ECS& ecs, Mod* mod = nullptr);
	~InterfaceFactory();
		
	void setMod(Mod* mod); // KN Note: Until we have a new mod interface, this will have to do.

	entt::handle createArrowButton(const std::string& name, ArrowShape shape, int width, int height, int x = 0, int y = 0);
	entt::handle createBar(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::handle createBattlescapeButton(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::handle createComboBox(const std::string& name, State* state, int width, int height, int x = 0, int y = 0, bool popupAboveButton = false);
	entt::handle createCursor(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::handle createFpsCounter(const std::string& name, int width, int height, int x, int y);
	entt::handle createFrame(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::handle createImageButton(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::handle createNumberText(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::handle createProgressBar(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::handle createScrollBar(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::handle createSlider(const std::string& name, int width, int height, int x = 0, int y = 0);

	struct CreateTextParams
	{
		std::string name;
		std::string text;
		int x;
		int y;
		int width;
		int height;

		TextHAlign align = TextHAlign::ALIGN_LEFT;
		TextVAlign verticalAlign = TextVAlign::ALIGN_TOP;
		bool wordWrap = false;

		SDL_Color* palette = nullptr;
		int firstColor = 0;
		int nColors = 256;

		std::string ruleCategory = "";
		std::string ruleID = "";

		entt::handle parent;
	};
	entt::handle createText(const CreateTextParams& params);

	struct CreateTextButtonParams
	{
		std::string name;
		std::string text;
		int x;
		int y;
		int width;
		int height;

		std::function<void(Action*)> onLeftClickCallback;
		std::function<void(Action*)> onRightClickCallback;

		SDL_Color* palette = nullptr;
		int firstColor = 0;
		int nColors = 256;

		std::string ruleID = "";
		std::string ruleCategory = "";

		entt::handle parent;
	};
	entt::handle createTextButton(const CreateTextButtonParams& params);

	entt::handle createTextEdit(const std::string& name, State* state, int width, int height, int x = 0, int y = 0);
	entt::handle createTextList(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::handle createToggleTextButton(const std::string& name, int width, int height, int x, int y);

	struct CreateWindowParams
	{
		std::string name;

		State* state = nullptr;

		int x;
		int y;
		int width;
		int height;
		WindowPopup popup = WindowPopup::POPUP_NONE;

		// KN TODO: perhaps use a palette handle instead of a pointer?
		SDL_Color* palette = nullptr;
		int firstColor = 0;
		int nColors = 256;

		int primaryColor = 0;

		std::string ruleID = "";
		std::string ruleCategory = "";

		Surface* background = nullptr;

		entt::handle parent;
	};

	[[deprecated("Use createWindow(const CreateWindowParams& params) instead")]] entt::handle createWindow(const std::string& name, State* state, int width, int height, int x = 0, int y = 0, WindowPopup popup = WindowPopup::POPUP_NONE);
	entt::handle createWindow(const CreateWindowParams& params);
};

} // namespace OpenXcom
