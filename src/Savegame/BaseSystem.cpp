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
 * Adds up the monthly maintenance of all the bases.
 * @return Total maintenance.
 */
int64_t getBasesMaintenanceCost()
{
	return getRegistry().totalBy<Base, int64_t>(&Base::getCraftMaintenance, &Base::getFacilityMaintenance, &Base::getPersonnelMaintenance);
}

/**
 * Returns if a certain item has been obtained, i.e. is present directly in the base stores.
 * Items in and on craft, in transfer, worn by soldiers, etc. are ignored!!
 * @param itemType Item ID.
 * @return Whether it's obtained or not.
 */
bool isItemInBaseStores(const std::string& itemType)
{
	for (const Base& xcomBase : getRegistry().list<const Base>())
	{
		if (xcomBase.getStorageItems()->getItem(itemType) > 0) { return true; }
	}
	return false;
}

/**
 * Returns if a certain facility has been built in any base.
 * @param facilityType facility ID.
 * @return Whether it's been built or not. If false, the facility has not been built in any base.
 */
bool BaseSystem::isFacilityBuilt(const std::string& facilityType)
{
	for (const Base& xcomBase : getRegistry().list<const Base>())
	{
		for (BaseFacility* fac : xcomBase.getFacilities())
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
bool isSoldierTypeHired(const std::string& soldierType)
{
	for (const Base& xcomBase : getRegistry().list<const Base>())
	{
		for (Soldier* soldier : xcomBase.getSoldiers())
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
