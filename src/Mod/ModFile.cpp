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
#include "ModFile.h"
#include "../Engine/FileMap.h"
#include "../Engine/Logger.h"
#include "../Engine/Options.h"

namespace OpenXcom
{



/// Predefined name for first loaded mod that have all original data
const std::string ModNameMaster = "master";
/// Predefined name for current mod that is loading rulesets.
const std::string ModNameCurrent = "current";


ModFile::ModFile()
{
}

ModFile::~ModFile()
{
}

bool ModFile::loadAll()
{
	// KN NOTE:
	// So this function sets up the modInfo offsets and size, but my question is, why can't we do that in Options? Why do we need to do it here?
	// The guts of this have mainly been copied from the old Mod class, which is(was?) a big huge mess. I needed this list for Lua, so I've moved
	// it here and cleaned it up as much as I could. I'm not too happy with it, especially since I have to use things like "friending" and
	// "const_cast", but I am copying the functionality without overly modifying it at this stage. I will come back to this later and clean it up.

	const FileMap::RSOrder& mods = FileMap::getRulesets();	//KN NOTE: FileMap is evil and should be nuked from orbit... it's the only way to be sure!

	_modData.clear();
	_modData.reserve(mods.size());

	std::set<std::string> usedModNames;
	usedModNames.insert(ModNameMaster);
	usedModNames.insert(ModNameCurrent);
	
	// calculated offsets and other things for all mods
	size_t offset = 0;
	for (size_t i = 0; mods.size() > i; ++i)
	{
		const std::string& modId = mods[i].name;
		if (usedModNames.insert(modId).second == false)
		{
			Log(LOG_WARNING) << "Duplicate mod found: " << modId;
			Log(LOG_WARNING) << "Skipping";
			continue;
		}

		//KN NOTE: ugly, I know, but until I get my head around the purposes of the offsets and sizes, I'm just going to run with this.
		ModInfo* modInfo = const_cast<ModInfo*>(&Options::getModInfos().at(modId));

		size_t size = modInfo->getReservedSpace();
		modInfo->setOffset(1000 * offset);	// KN NOTE: WTF?
		modInfo->setSize(1000 * size);
		modInfo->setRulesetFiles(mods[i].files);

		_modData.push_back(modInfo);

		offset += size;
	}

	return true;
}

} // namespace OpenXcom
