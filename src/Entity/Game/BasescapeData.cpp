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
#include "BasescapeData.h"
#include <ranges>
#include "Base.h"
#include "../Common/Index.h"

namespace OpenXcom {

void BasescapeSystem::updateListeners()
{
	for(std::function<void()>& callback : _onUpdateListeners | std::views::elements<1>)
	{
		callback();
	}
}

size_t BasescapeSystem::connectListener(std::function<void()> listener)
{
	size_t index = _onUpdateListeners.size();
	_onUpdateListeners[_onUpdateListeners.size()] = std::move(listener);
	return index;
}

/**
 * @brief Gets a handle to the currently selected base
 * @return a handle to the currently selected base
 */
entt::handle BasescapeSystem::getSelectedBase()
{
	size_t index = _gameHandle.get<BasescapeData>().selectedBaseIndex; // copy
	std::ranges::range auto view = _registry.view<Index, Base>().each();
	auto matchingIndex = [index](const auto& each) { return std::get<1>(each).index == index; };
	auto findResult = std::ranges::find_if(view, matchingIndex); ;
	return findResult != view.end() ? entt::handle(_registry, std::get<0>(*findResult)) : entt::handle();
}

/**
 * @brief tries to set the selected base index according to its visible
 * @param index the index to set
 * @return true of the index was changed, false otherwise
 */
bool BasescapeSystem::trySetSelectedBaseByVisbleIndex(size_t index)
{
	BasescapeData basescapeData = _gameHandle.get<BasescapeData>(); // copy
	basescapeData.visibleBasesOffset += index;
	if (index < _registry.storage<Base>().size())
	{
		_gameHandle.replace<BasescapeData>(basescapeData);
		updateListeners();
		return true;
	}
	return false;
}

/**
 * @brief Sets the selected base index acording to its absolute index, without regard to validity.
 * @param index the index to set.
 */
void BasescapeSystem::setSelectedBaseIndex(size_t index)
{
	BasescapeData basescapeData = _gameHandle.get<BasescapeData>(); // copy
	basescapeData.selectedBaseIndex = index;
	_gameHandle.replace<BasescapeData>(basescapeData);
}

/**
 * @brief Tries to increment the visible base index by one
 * @return true if the offset was incremented, false otherwise.
 */
bool BasescapeSystem::tryIncrementVisibleBaseOffset()
{
	BasescapeData basescapeData = _gameHandle.get<BasescapeData>();
	if (basescapeData.visibleBasesOffset < _registry.storage<Base>().size() - BasescapeData::MAX_VISIBLE_BASES)
	{
		++basescapeData.visibleBasesOffset;
		_gameHandle.replace<BasescapeData>(basescapeData);
		return true;
	}
	return false;
}

/**
 * @brief Tries to decrement the visible base index by one
 * @return true if the offset was incremented, false otherwise
 */
bool BasescapeSystem::tryDecrementVisibleBaseOffset()
{
	BasescapeData basescapeData = _gameHandle.get<BasescapeData>();
	if (basescapeData.visibleBasesOffset > 0)
	{
		--basescapeData.visibleBasesOffset;
		_gameHandle.replace<BasescapeData>(basescapeData);
		return true;
	}
	return false;
}

}

