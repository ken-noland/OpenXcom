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
#include <vector>

namespace OpenXcom
{

struct HierarchyComponent
{
	std::vector<entt::handle> _children;
};

class HierarchySystem
{
public:
	HierarchySystem() = default;
	~HierarchySystem() = default;

	/// Adds a child entity to a parent entity.
	void addChild(entt::handle parent, entt::handle child);

	/// Removes a child entity from a parent entity. (Does not destroy the child entity.)
	void removeChild(entt::handle parent, entt::handle child);


	using VisitFunction = std::function<void(entt::handle)>;

	/// Visit each child entity. Only the first level of children is visited.
	void visit(entt::handle entity, VisitFunction func);

	/// Visit each child entity. If the child entity contains a HierarchyComponent, it will be visited recursively.
	void visitRecursive(entt::handle entity, VisitFunction func);
};

} // namespace OpenXcom
