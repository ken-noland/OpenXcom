#pragma once
/*
 * Copyright 2024-2024 OpenXcom Developers.
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
#include <ranges>
#include <string>
#include <entt/entt.hpp>
#include "../../Savegame/Base.h"

namespace OpenXcom {

struct BasescapeData
{
	// the position of the currently selected base among the entire base index.
	size_t selectedBaseIndex;
	// the posiitons the visible list of bases is offset from the origin.
	size_t visibleBasesOffset;

	size_t getSelectedBaseVisibleIndex() const { return selectedBaseIndex - visibleBasesOffset; }

	static constexpr size_t MAX_VISIBLE_BASES = 8;

	static constexpr std::string_view NODE_NAME = "basescapeData";
};

class BasescapeSystem
{
	entt::registry& _registry;
	entt::handle _gameHandle;

	std::unordered_map<size_t, std::function<void()>> _onUpdateListeners;

	void updateListeners();

public:
	BasescapeSystem(entt::registry& registry, entt::handle gameHandle) : _registry(registry), _gameHandle(gameHandle) { }

	// connects a listner to recieve updates when the basecape data changes.
	[[nodiscard]] size_t connectListener(std::function<void()> listener);
	// removes a connected listener.
	void disconnectListener(size_t id) { _onUpdateListeners.erase(id); }

	// gets a view of the currently visible base handles
	[[nodiscard]] inline auto getVisibleBases()
	{
		BasescapeData basescapeData = _gameHandle.get<BasescapeData>();
		return _registry.view<Base>() | std::views::drop(basescapeData.visibleBasesOffset)
			| std::views::take(BasescapeData::MAX_VISIBLE_BASES)
			| std::views::transform([&](entt::entity baseId) { return entt::handle(_registry, baseId); });
	}

	// gets a handle to the currently selected base
	[[nodiscard]] entt::handle getSelectedBase();

	// gets the basecape data;
	[[nodiscard]] inline BasescapeData getBasescapeData() { return _gameHandle.get<BasescapeData>(); }

	// gets the total number of bases
	[[nodiscard]] inline size_t getBaseCount() { return _registry.storage<Base>().size(); }

	bool trySetSelectedBaseByVisbleIndex(size_t index);

	void setSelectedBaseIndex(size_t index);

	bool tryIncrementVisibleBaseOffset();

	bool tryDecrementVisibleBaseOffset();
};

}

namespace YAML
{

template<>
struct convert<OpenXcom::BasescapeData> {
	static Node encode(const OpenXcom::BasescapeData& basescapeData) {
		Node node;
		node["selectedBaseIndex"] = basescapeData.selectedBaseIndex;
		node["visibleBasesOffset"] = basescapeData.visibleBasesOffset;
		return node;
	}

	static bool decode(const Node& node, OpenXcom::BasescapeData& basescapeData) {
		if (!node.IsMap() || node.size() != 2) {
			return false;
		}

		basescapeData.selectedBaseIndex  = node["selectedBaseIndex"].as<size_t>();
		basescapeData.visibleBasesOffset = node["visibleBasesOffset"].as<size_t>();
		return true;
	}
};
}
