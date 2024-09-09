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
#include "Inspector.h"
#include "InspectorApp.h"
#include "InspectorFrame.h"

#include "../Engine/Game.h"

namespace OpenXcom
{


Inspector::Inspector()
{

}

Inspector::~Inspector()
{
}

void Inspector::create()
{
	if (!wxTheApp)
	{
		_app = new InspectorApp(); // Create a new wxApp instance
		wxApp::SetInstance(_app);  // Create a new wxApp instance
		wxEntryStart(0, nullptr);  // Initialize wxWidgets
		wxTheApp->CallOnInit();    // Initialize the app
	}
	_frame = new InspectorFrame("OpenXcom Inspector", getGame()->getECS()); // Create the inspector window
	show();
}

void Inspector::destroy()
{
	if (_frame)
	{
		_frame->Destroy();
		_frame = nullptr;
	}
	wxEntryCleanup(); // Clean up wxWidgets
}

void Inspector::show()
{
	if (!_isShown)
	{
		_frame->Show(true); // Show the window
		_isShown = true;
	}
}

void Inspector::hide()
{
	if (_isShown)
	{
		_frame->Show(false); // Hide the window
		_isShown = false;
	}
}

} // namespace OpenXcom
