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

namespace OpenXcom
{

InspectorFrame::InspectorFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600))
{
	Bind(wxEVT_CLOSE_WINDOW, &InspectorFrame::OnClose, this);

	SetupUI();
}


void InspectorFrame::SetupUI()
{
	// Create a notebook (tab control)
	notebook = new wxNotebook(this, wxID_ANY);

	// Tab 1: Entity View
	wxPanel* entityPanel = new wxPanel(notebook);
	entityTree = new wxTreeCtrl(entityPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE);
	wxBoxSizer* entitySizer = new wxBoxSizer(wxVERTICAL);
	entitySizer->Add(entityTree, 1, wxEXPAND);
	entityPanel->SetSizer(entitySizer);
	notebook->AddPage(entityPanel, "Entities");

	// Tab 2: System View
	wxPanel* systemPanel = new wxPanel(notebook);
	systemList = new wxListBox(systemPanel, wxID_ANY);
	wxBoxSizer* systemSizer = new wxBoxSizer(wxVERTICAL);
	systemSizer->Add(systemList, 1, wxEXPAND);
	systemPanel->SetSizer(systemSizer);
	notebook->AddPage(systemPanel, "Systems");

	// Populate initial data
	PopulateEntityTree();

	// Set the notebook as the main layout
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(notebook, 1, wxEXPAND);
	SetSizer(mainSizer);
}

void InspectorFrame::PopulateEntityTree()
{
	wxTreeItemId root = entityTree->AddRoot("Entities");

	// Example entities and components
	wxTreeItemId entity1 = entityTree->AppendItem(root, "Entity 1");
	entityTree->AppendItem(entity1, "Component: Health");
	entityTree->AppendItem(entity1, "Component: Position");

	wxTreeItemId entity2 = entityTree->AppendItem(root, "Entity 2");
	entityTree->AppendItem(entity2, "Component: AI");
	entityTree->AppendItem(entity2, "Component: Renderer");

	entityTree->ExpandAll();
}

void InspectorFrame::OnClose(wxCloseEvent& event)
{
	// Hide the window instead of destroying it
	this->Show(false);
	event.Skip(); // Allow the event to propagate further if needed
}


}
