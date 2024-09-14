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
#include "InspectorFrame.h"
#include "EntityPanel.h"
#include "../Entity/Engine/ECS.h"



namespace OpenXcom
{

InspectorFrame::InspectorFrame(const wxString& title, ECS& ecs) : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)), _ecs(ecs)
{
	Bind(wxEVT_CLOSE_WINDOW, &InspectorFrame::OnClose, this);

	SetupUI();
}


void InspectorFrame::SetupUI()
{
	// Create a notebook (tab control)
	_notebook = new wxNotebook(this, wxID_ANY);

	// Tab 1: Entity View
	wxPanel* entityPanel = new EntityPanel(_notebook, _ecs.getRegistry().raw());
	_notebook->AddPage(entityPanel, "Entities");

	// Tab 2: System View
	wxPanel* systemPanel = new wxPanel(_notebook);
	_systemList = new wxListBox(systemPanel, wxID_ANY);
	wxBoxSizer* systemSizer = new wxBoxSizer(wxVERTICAL);
	systemSizer->Add(_systemList, 1, wxEXPAND);
	systemPanel->SetSizer(systemSizer);
	_notebook->AddPage(systemPanel, "Systems");

	// Set the notebook as the main layout
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(_notebook, 1, wxEXPAND);
	SetSizer(mainSizer);
}




void InspectorFrame::OnClose(wxCloseEvent& event)
{
	// Hide the window instead of destroying it
	this->Show(false);
	event.Skip(); // Allow the event to propagate further if needed
}



}
