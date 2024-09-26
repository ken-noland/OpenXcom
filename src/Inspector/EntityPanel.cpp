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
#include "InspectorFrame.h"

#include "../Entity/Common/Name.h"

#include <simplerttr.h>

#include "../Entity/Common/RTTR.h"


namespace OpenXcom
{

wxBEGIN_EVENT_TABLE(EntityPanel, wxPanel)
	EVT_SEARCHCTRL_SEARCH_BTN(ID_SearchCtrl, EntityPanel::onSearch)
	EVT_TREE_SEL_CHANGED(ID_EntityTree, EntityPanel::onEntitySelected)
	EVT_TIMER(ID_RefreshTimer, EntityPanel::onTimer)
wxEND_EVENT_TABLE()

// Custom data to attach to tree items
class EntityComponentData : public wxTreeItemData
{
public:
	entt::handle _entityHandle;

	SimpleRTTR::Type _componentType;
	GetComponentFunc _getComponentFunc;

	EntityComponentData(entt::handle entityHandle, const SimpleRTTR::Type& componentType = SimpleRTTR::Type::invalid_type(), GetComponentFunc getComponentFunc = nullptr)
		: _entityHandle(entityHandle), _componentType(componentType), _getComponentFunc(getComponentFunc) {}
};


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

	
	// Create a wxPropertyGrid to display properties
	_entityDetailGrid = new CustomPropertyGrid(_entityDetailPanel);
	detailSizer->Add(_entityDetailGrid, 1, wxALL | wxEXPAND, 0);
	_entityDetailGrid->Hide(); // Hide initially

	_entityDetailLabel = new wxStaticText(_entityDetailPanel, wxID_ANY, "Select an entity or component to view details.");
	detailSizer->Add(_entityDetailLabel, 1, wxALL | wxEXPAND, 10);

	_entityDetailPanel->SetSizer(detailSizer);

	// Split the window horizontally
	splitter->SplitVertically(entityPanel, _entityDetailPanel);
	splitter->SetSashGravity(0.3); // 30% of space for tree view

	// Add splitter to the main sizer
	mainSizer->Add(splitter, 1, wxEXPAND);

	// Set sizer for the main panel
	this->SetSizer(mainSizer);

	showInitialMessage();
}

void EntityPanel::showInitialMessage()
{
	showMessage("Select an entity or component to view details.");
}

void OpenXcom::EntityPanel::showMessage(const std::string& message)
{
	_entityDetailGrid->Hide(); // Hide initially
	_entityDetailLabel->SetLabelText(message);
	_entityDetailLabel->Show();
}

void EntityPanel::showEntityDetails(EntityComponentData* data)
{
//	_entityDetailGrid->Clear();

	NameComponent* nameComponent = data->_entityHandle.try_get<NameComponent>();
	std::string entityName = "<unnamed>";
	if (nameComponent)
	{
		std::string entityName = data->_componentType.name();
	}

//	_entityDetailGrid->Append(new wxStringProperty("Entity", wxPG_LABEL, entityName));
}

void EntityPanel::showComponentDetails(EntityComponentData* data)
{
	_entityDetailGrid->Clear();

	std::string componentName = data->_componentType.name();

	_entityDetailGrid->Add("Component", [&componentName](wxWindow* parent) { return new wxStaticText(parent, wxID_ANY, componentName); });

	if (data->_componentType != SimpleRTTR::Type::invalid_type())
	{
		// loop through the properties
		for (const SimpleRTTR::Property& property : data->_componentType.properties())
		{
			// get the property type
			SimpleRTTR::Type propertyType = property.type();
			std::string propertyName = property.name();

			//TEST
			std::string message = "Unable to create property sheet for type \"" + propertyType.name() + "\"";
			_entityDetailGrid->Add(propertyName, [&](wxWindow* parent)
				{ return _typePropertyMapping.createProperty(parent, data->_entityHandle, data->_componentType, property); });
			//	{ return new wxStaticText(parent, wxID_ANY, message); });



			//wxPGProperty* prop = _typePropertyMapping.createProperty(data->_entityHandle, data->_getComponentFunc, data->_componentType, property);
			//if (prop)
			//{
			//	//this needs to be changed around a bit
			//}
			//else
			//{
			//}
		}
	}

	_entityDetailLabel->Hide();
	_entityDetailGrid->Show();

	_entityDetailPanel->Layout(); // Refresh layout
}

void EntityPanel::setupEntityCallbacks()
{
	_registry.on_construct<entt::entity>().connect<&EntityPanel::onEntityAdded>(this);
}



void EntityPanel::populateEntityTree()
{
	_needsRefresh = false;

	// remove the entities that were removed
	for (entt::entity entity : _entitiesRemoved)
	{
		wxTreeItemId item = _entityToTreeItemMap[entity];
		_entityTree->Delete(item);
		_entityToTreeItemMap.erase(entity);

		_needsRefresh = true;
	}
	_entitiesRemoved.clear();

	// add the entities that were added
	for (entt::entity entity : _entitiesAdded)
	{
		wxTreeItemId rootItem = _entityTree->GetRootItem();
		bool rootHadNoChildren = !_entityTree->ItemHasChildren(rootItem);

		NameComponent* name = _registry.try_get<NameComponent>(entity);
		std::string nameStr = name != nullptr ? name->name : "<unnamed>";

		wxTreeItemId entityItem = _entityTree->AppendItem(rootItem, nameStr.c_str());
		_entityTree->Expand(entityItem);

		_entityTree->SetItemData(entityItem, new EntityComponentData(entt::handle(_registry, entity)));

		// now lets visit each component
		for (const entt::registry::iterable::value_type& storage : _registry.storage())
		{
			entt::basic_sparse_set<entt::entity>& entities = storage.second;
			if (entities.contains(entity))
			{
				// get the type information
				std::string typeName = std::string(entities.type().name());
				SimpleRTTR::Type type = SimpleRTTR::types().get_type(typeName);

				wxTreeItemId componentItem;
				if (type != SimpleRTTR::Type::invalid_type())
				{
					componentItem = _entityTree->AppendItem(entityItem, type.name());

					// This may look a bit funny, but basically since there is no way to get a reference or pointer to a component
					// without knowing the component type, we have to work around this by pre-registering a function which strips
					// all the type data away so we can then use the run time type information to drive the rest of the property
					// pages. This means that each type we want to have property pages for, we need to register a Meta key/value pair
					// which uses GetComponentFuncName as the key and a pointer to a function with the signature of GetComponentFunc.
					// From there, we can extract the properties and display them in the property grid.
					if (type.meta().has(GetComponentFuncName))
					{
						const SimpleRTTR::Meta& getComponentMeta = type.meta().get(GetComponentFuncName);
						GetComponentFunc getComponentFunc = getComponentMeta.value().get_as<GetComponentFunc>();

						_entityTree->SetItemData(componentItem, new EntityComponentData(entt::handle(_registry, entity), type, getComponentFunc));
					}
					else
					{
						_entityTree->SetItemData(componentItem, new EntityComponentData(entt::handle(_registry, entity), type));
					}
				}
				else
				{
					componentItem = _entityTree->AppendItem(entityItem, entities.type().name().data());
				}
			}
		}

		// If the root had no children before, expand it now
		if (rootHadNoChildren)
		{
			_entityTree->Expand(rootItem);
		}

		_entityToTreeItemMap[entity] = entityItem;
		_needsRefresh = true;
	}
	_entitiesAdded.clear();

	if (_needsRefresh)
	{
		_entityTree->Refresh();
	}
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
	wxTreeItemId selectedItem = _entityTree->GetSelection();
	if (selectedItem.IsOk())
	{
		EntityComponentData* data = dynamic_cast<EntityComponentData*>(_entityTree->GetItemData(selectedItem));
		if (data)
		{
			if (data->_componentType != SimpleRTTR::Type::invalid_type())
			{
				if (data->_getComponentFunc)
				{
					// we have a component that is properly registered, so display it's property pages
					showComponentDetails(data);
				}
				else
				{
					// we have a component, but it isn't properly registered(likely missing GetComponentFuncName Metadata)
					showMessage("Unable to get component data for component type \"" + data->_componentType.name() + "\"");
				}
			}
			else
			{
				// we are displaying an entity
				showEntityDetails(data);
			}
		}
		else
		{
			// hmm... no data
			showMessage("Unable to get data for selection");
		}
	}
	else
	{
		// no entity selected
		showInitialMessage();
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
