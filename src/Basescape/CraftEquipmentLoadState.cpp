/*
* Copyright 2010-2015 OpenXcom Developers.
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
#include "CraftEquipmentLoadState.h"
#include "CraftEquipmentState.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/ToggleTextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/SavedGame.h"
#include "../Entity/Interface/Interface.h"

namespace OpenXcom
{

/**
* Initializes all the elements in the Load Craft Loadout window.
*/
CraftEquipmentLoadState::CraftEquipmentLoadState(CraftEquipmentState* parent)
	: State("CraftEquipmentLoadState", false), _parent(parent)
{
	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();

	// Create objects
	_window = factory.createWindow("craftEquipmentLoad", this, 240, 136, 40, 36 + 1, WindowPopup::POPUP_BOTH);
	_txtTitle = new Text(230, 16, 45, 44+3);
	_lstLoadout = new TextList(208, 80, 48, 60);
	_btnOnlyAdd = new ToggleTextButton(100, 16, 56, 148);
	_btnCancel = new TextButton(100, 16, 164, 148);

	// Set palette
	setInterface("craftEquipmentLoad");

	add(_window, "window", "craftEquipmentLoad");
	add(_txtTitle, "text", "craftEquipmentLoad");
	add(_lstLoadout, "list", "craftEquipmentLoad");
	add(_btnOnlyAdd, "button", "craftEquipmentLoad");
	add(_btnCancel, "button", "craftEquipmentLoad");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "craftEquipmentLoad");

	_txtTitle->setAlign(TextHAlign::ALIGN_CENTER);
	_txtTitle->setText(tr("STR_LOAD_CRAFT_LOADOUT_TEMPLATE"));

	_lstLoadout->setColumns(1, 192);
	_lstLoadout->setSelectable(true);
	_lstLoadout->setBackground(_window);

	_lstLoadout->setMargin(8);
	_lstLoadout->onMouseClick((ActionHandler)&CraftEquipmentLoadState::lstLoadoutClick);

	_btnOnlyAdd->setText(tr("STR_ADD_ON_TOP"));

	_btnCancel->setText(tr("STR_CANCEL_UC"));
	_btnCancel->onMouseClick((ActionHandler)&CraftEquipmentLoadState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&CraftEquipmentLoadState::btnCancelClick, Options::keyCancel);

	for (int i = 0; i < SavedGame::MAX_CRAFT_LOADOUT_TEMPLATES; ++i)
	{
		ItemContainer* item = getGame()->getSavedGame()->getGlobalCraftLoadout(i);
		if (item->getContents().empty())
		{
			_lstLoadout->addRow(1, tr("STR_EMPTY_SLOT_N").arg(i + 1).c_str());
		}
		else
		{
			const std::string& itemName = getGame()->getSavedGame()->getGlobalCraftLoadoutName(i);
			if (itemName.empty())
			{
				_lstLoadout->addRow(1, tr("STR_UNNAMED_SLOT_N").arg(i + 1).c_str());
			}
			else
			{
				_lstLoadout->addRow(1, itemName.c_str());
			}
		}
	}
}

/**
*
*/
CraftEquipmentLoadState::~CraftEquipmentLoadState()
{

}

/**
* Returns to the previous screen.
* @param action Pointer to an action.
*/
void CraftEquipmentLoadState::btnCancelClick(Action *)
{
	getGame()->popState();
}

/**
* Loads the template and returns to the previous screen.
* @param action Pointer to an action.
*/
void CraftEquipmentLoadState::lstLoadoutClick(Action *)
{
	getGame()->popState();
	bool addOnTop = _btnOnlyAdd->getPressed() || getGame()->isCtrlPressed();
	_parent->loadGlobalLoadout(_lstLoadout->getSelectedRow(), addOnTop);
}

}
