#include "Inspector.h"
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

#include <SDL/SDL_keysym.h>

int SDLKeyToWxKey(int sdlKey) {
	// Map SDL key codes to wxWidgets key codes
	switch (sdlKey) {
	case SDLK_RETURN:
		return WXK_RETURN;
	case SDLK_BACKSPACE:
		return WXK_BACK;
	// Add more key mappings as needed
	default:
		return sdlKey; // Fallback to direct mapping if possible
	}
}

wxKeyEvent ConvertSDLEventToWx(const SDL_Event& sdlEvent) {
	// Create a wxKeyEvent based on SDL event data
	int keyCode = SDLKeyToWxKey(sdlEvent.key.keysym.sym);
	wxKeyEvent wxEvent(sdlEvent.type == SDL_KEYDOWN ? wxEVT_KEY_DOWN : wxEVT_KEY_UP);
	wxEvent.SetId(keyCode);
//	wxEvent.SetKeyCode(keyCode);
	// Set additional event properties like modifiers here if needed

	return wxEvent;
}

void Inspector::handleEvent(const SDL_Event& event)
{
	if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
		wxKeyEvent keyEvent = ConvertSDLEventToWx(event);
		wxPostEvent(wxTheApp->GetTopWindow(), keyEvent);
	}
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
