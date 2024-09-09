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
#include "EntityPanel.h"
#include "../Entity/Common/Name.h"

#include <simplerttr.h>

namespace OpenXcom
{

wxBEGIN_EVENT_TABLE(EntityPanel, wxPanel)
	EVT_SEARCHCTRL_SEARCH_BTN(ID_SearchCtrl, EntityPanel::onSearch)
	EVT_TREE_SEL_CHANGED(ID_EntityTree, EntityPanel::onEntitySelected)
	EVT_TIMER(ID_RefreshTimer, EntityPanel::onTimer)
wxEND_EVENT_TABLE()


EntityPanel::EntityPanel(wxWindow* parent, entt::registry& registry)
	: wxPanel(parent), _registry(registry), _timer(this, ID_RefreshTimer)
{
	setupUI();
	setupEntityCallbacks();
	
	// update every half second
	_timer.Start(500);
}


void EntityPanel::setupUI()
{
	// Main vertical sizer for the panel
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	// Split window for tree view and detail view
	wxSplitterWindow* splitter = new wxSplitterWindow(this, wxID_ANY);

	// Create a panel to hold the search control and tree view (left side)
	wxPanel* entityPanel = new wxPanel(splitter, wxID_ANY);
	wxBoxSizer* entitySizer = new wxBoxSizer(wxVERTICAL);

	// Search control (wxSearchCtrl) for searching entities
	_searchCtrl = new wxSearchCtrl(entityPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	_searchCtrl->ShowSearchButton(true); // Enable the search button
	_searchCtrl->ShowCancelButton(true); // Enable the cancel button
	_searchCtrl->Enable(true);           // Enable the search control

	// Tree view for displaying entities
	_entityTree = new wxTreeCtrl(entityPanel, ID_EntityTree, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE);
	_entityTree->AddRoot("Entities");

	// Add search control and tree view to the sizer of the left panel
	entitySizer->Add(_searchCtrl, 0, wxEXPAND | wxALL, 5); // Add wxSearchCtrl at the top
	entitySizer->Add(_entityTree, 1, wxEXPAND);

	// Set sizer for the entity panel
	entityPanel->SetSizer(entitySizer);

	// Panel for entity details on the right side
	_entityDetailPanel = new wxPanel(splitter, wxID_ANY);
	wxBoxSizer* detailSizer = new wxBoxSizer(wxVERTICAL);
	wxStaticText* detailLabel = new wxStaticText(_entityDetailPanel, wxID_ANY, "Select an entity to view details.");
	detailSizer->Add(detailLabel, 1, wxALL | wxEXPAND, 10);
	_entityDetailPanel->SetSizer(detailSizer);

	// Split the window horizontally
	splitter->SplitVertically(entityPanel, _entityDetailPanel);
	splitter->SetSashGravity(0.3); // 30% of space for tree view

	// Add splitter to the main sizer
	mainSizer->Add(splitter, 1, wxEXPAND);

	// Set sizer for the main panel
	this->SetSizer(mainSizer);
}

void EntityPanel::setupEntityCallbacks()
{
	_registry.on_construct<entt::entity>().connect<&EntityPanel::onEntityAdded>(this);
}



void EntityPanel::populateEntityTree()
{
	// remove the entities that were removed
	for (entt::entity entity : _entitiesRemoved)
	{
		wxTreeItemId item = _entityToTreeItemMap[entity];
		_entityTree->Delete(item);
		_entityToTreeItemMap.erase(entity);
	}
	_entitiesRemoved.clear();

	// add the entities that were added
	for (entt::entity entity : _entitiesAdded)
	{
		NameComponent* name = _registry.try_get<NameComponent>(entity);
		std::string nameStr = name ? name->name : "<unnamed>";
		wxTreeItemId entityItem = _entityTree->AppendItem(_entityTree->GetRootItem(), nameStr.c_str());

		// now lets visit each component
		for (const entt::registry::iterable::value_type& storage : _registry.storage())
		{
			if (storage.second.contains(entity))
			{
				// get the type information
				std::string typeName = std::string(storage.second.type().name());
				SimpleRTTR::Type type = SimpleRTTR::Types().GetType(typeName);

				if (type != SimpleRTTR::Type::InvalidType())
				{
					_entityTree->AppendItem(entityItem, type.Name());
				}
				else
				{
					_entityTree->AppendItem(entityItem, storage.second.type().name().data());
				}
			}
		}

		_entityToTreeItemMap[entity] = entityItem;
	}
	_entitiesAdded.clear();

	_entityTree->ExpandAll();
}

void EntityPanel::onTimer(wxTimerEvent& event)
{
	populateEntityTree();
}


void EntityPanel::onSearch(wxCommandEvent& event)
{
	wxString searchText = _searchCtrl->GetValue();
	// Implement search logic for the entity tree here.
	//wxLogMessage("Search for: %s", searchText);
}

void EntityPanel::onEntitySelected(wxTreeEvent& event)
{
	wxTreeItemId selected = _entityTree->GetSelection();
	if (selected.IsOk())
	{
		//wxLogMessage("Entity selected: %s", _entityTree->GetItemText(selected));
		// Update the right-side panel with entity details.
	}
}

void EntityPanel::onEntityAdded(entt::registry& registry, entt::entity entity)
{
	_entitiesAdded.push_back(entity);
}

void EntityPanel::onEntityRemoved(entt::registry& registry, entt::entity entity)
{
	_entitiesRemoved.push_back(entity);
}

} // namespace OpenXcom
