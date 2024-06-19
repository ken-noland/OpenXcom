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
#include <numeric>
#include <ranges>
#include "BaseSystem.h"
#include "Base.h"
#include "BaseFacility.h"
#include "ItemContainer.h"
#include "Soldier.h"
#include "../Engine/Registry.h"
#include "../Mod/RuleSoldier.h"

namespace OpenXcom::BaseSystem
{

/**
 * @brief Gets the index of a given base in the registry by its id.
 * @param registry the global entity registry
 * @param baseId the base Id to search for
 * @return the index of the base entity, or -1 if it is not found.
*/
static int getBaseIndex(const entt::registry& registry, entt::entity baseId)
{
	auto baseView = registry.view<Base>();
	auto findIterator = std::find(baseView.begin(), baseView.end(), baseId);
	return findIterator != baseView.end() ? std::distance(baseView.begin(), findIterator) : -1;
}

/**
 * Adds up the monthly maintenance of all the bases.
 * @return Total maintenance.
 */
static int64_t getBasesMaintenanceCost()
{
	return getRegistry().totalBy<Base, int64_t>(&Base::getCraftMaintenance, &Base::getFacilityMaintenance, &Base::getPersonnelMaintenance);
}

/**
 * Returns if a certain item has been obtained, i.e. is present directly in the base stores.
 * Items in and on craft, in transfer, worn by soldiers, etc. are ignored!!
 * @param itemType Item ID.
 * @return Whether it's obtained or not.
 */
static bool isItemInBaseStores(const entt::registry& registry, const std::string& itemType)
{
	for (const auto&& [id, base] : registry.view<Base>().each())
	{
		if (base.getStorageItems()->getItem(itemType) > 0)
			return true;
	}
	return false;
}

/**
 * Returns if a certain facility has been built in any base.
 * @param facilityType facility ID.
 * @return Whether it's been built or not. If false, the facility has not been built in any base.
 */
static bool BaseSystem::isFacilityBuilt(const entt::registry& registry, const std::string& facilityType)
{
	for (const auto&& [id, base] : registry.view<const Base>().each())
	{
		for (BaseFacility* fac : base.getFacilities())
		{
			if (fac->getBuildTime() == 0 && fac->getRules()->getType() == facilityType)
			{
				return true;
			}
		}
	}
	return false;
}

/**
 * Returns if a certain soldier type has been hired in any base.
 * @param soldierType soldier type ID.
 * @return Whether it's been hired (and arrived already) or not.
 */
static bool isSoldierTypeHired(const entt::registry& registry, const std::string& soldierType)
{
	for (const auto&& [id, base] : registry.view<const Base>().each())
	{
		for (Soldier* soldier : base.getSoldiers())
		{
			if (soldier->getRules()->getType() == soldierType)
			{
				return true;
			}
		}
	}
	return false;
}

}
