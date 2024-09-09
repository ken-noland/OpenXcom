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
#include "WxInclude.h"
#include <entt/entt.hpp>

namespace OpenXcom
{

class EntityPanel : public wxPanel
{
public:
	EntityPanel(wxWindow* parent, entt::registry& registry);

private:
	entt::registry& _registry;

	wxSearchCtrl* _searchCtrl;
	wxTreeCtrl* _entityTree;
	wxPanel* _entityDetailPanel;

	//between updates, these are the entities that have been added
	//  KN TODO: maybe we should just have a single list with a flag for added/removed/updated so that order of operations remains consistent?
	std::vector<entt::entity> _entitiesAdded;
	std::vector<entt::entity> _entitiesRemoved;

	wxTimer _timer;
	bool _needsRefresh;

	std::unordered_map<entt::entity, wxTreeItemId> _entityToTreeItemMap;

	void setupUI();
	void setupEntityCallbacks();

	void populateEntityTree();

	void onEntityAdded(entt::registry& registry, entt::entity entity);
	void onEntityRemoved(entt::registry& registry, entt::entity entity);

	void onTimer(wxTimerEvent& event);

	void onSearch(wxCommandEvent& event);
	void onEntitySelected(wxTreeEvent& event);

	wxDECLARE_EVENT_TABLE();
};

enum
{
	ID_RefreshTimer = 1,
	ID_SearchCtrl = 1001,
	ID_EntityTree = 1002
};

} // namespace OpenXcom
