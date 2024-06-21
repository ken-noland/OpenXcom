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
#include "../Engine/Surface.h"

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

namespace OpenXcom
{

class InterfaceFactory
{
protected:
	entt::registry& _registry;
	SurfaceFactory& _surfaceFactory;

public:
	InterfaceFactory(entt::registry& registry, SurfaceFactory& sf);
	~InterfaceFactory();

	entt::entity createArrowButton(const std::string& name, ArrowShape shape, int width, int height, int x = 0, int y = 0);
	entt::entity createBar(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::entity createBattlescapeButton(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::entity createComboBox(const std::string& name, State* state, int width, int height, int x = 0, int y = 0, bool popupAboveButton = false);
	entt::entity createCursor(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::entity createFpsCounter(const std::string& name, int width, int height, int x, int y);
	entt::entity createFrame(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::entity createImageButton(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::entity createNumberText(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::entity createProgressBar(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::entity createScrollBar(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::entity createSlider(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::entity createText(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::entity createTextButton(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::entity createTextEdit(const std::string& name, State* state, int width, int height, int x = 0, int y = 0);
	entt::entity createTextList(const std::string& name, int width, int height, int x = 0, int y = 0);
	entt::entity createToggleTextButton(const std::string& name, int width, int height, int x, int y);
	entt::entity createWindow(const std::string& name, State* state, int width, int height, int x = 0, int y = 0, WindowPopup popup = WindowPopup::POPUP_NONE);
};

} // namespace OpenXcom
