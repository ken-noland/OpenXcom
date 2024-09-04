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
#include "Hierarchical.h"

namespace OpenXcom
{

void HierarchySystem::addChild(entt::handle parent, entt::handle child)
{
	HierarchyComponent& hierarchyComponent = parent.get<HierarchyComponent>();
	hierarchyComponent._children.push_back(child);
}

void HierarchySystem::removeChild(entt::handle parent, entt::handle child)
{
	HierarchyComponent& hierarchyComponent = parent.get<HierarchyComponent>();
	hierarchyComponent._children.erase(std::remove(hierarchyComponent._children.begin(), hierarchyComponent._children.end(), child), hierarchyComponent._children.end());
}

void HierarchySystem::visit(entt::handle entity, VisitFunction func)
{
	HierarchyComponent* hierarchyComponent = entity.try_get<HierarchyComponent>();
	if (hierarchyComponent)
	{
		for (entt::handle child : hierarchyComponent->_children)
		{
			func(child);
		}
	}
}

void HierarchySystem::visitRecursive(entt::handle entity, VisitFunction func)
{
	HierarchyComponent* hierarchyComponent = entity.try_get<HierarchyComponent>();
	if (hierarchyComponent)
	{
		HierarchyComponent& hierarchyComponent = entity.get<HierarchyComponent>();
		for (entt::handle child : hierarchyComponent._children)
		{
			func(child);
			visitRecursive(child, func);
		}
	}
}

} // namespace OpenXcom
