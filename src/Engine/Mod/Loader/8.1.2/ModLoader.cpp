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
#include "ModLoader.h"
#include "../../Mod.h"
#include "../../../Logger.h"
#include "../../../YamlFile.h"
#include "../../../Utility/RTTR.h"
#include "../../../Filesystem/CompositeFileSystem.h"

namespace OpenXcom
{

namespace ModLoader_8_1_2
{

// register the structs for serialization
SIMPLERTTR
{
	SimpleRTTR::registration().type<SoundDefinition>()
		.meta("Serialize", ObjectSerialize::ALWAYS)
		.property(&SoundDefinition::type, "type")
			.meta("Serialize", PropertySerialize::ALWAYS)
		.property(&SoundDefinition::file, "file")
			.meta("Serialize", PropertySerialize::ALWAYS)
		.property(&SoundDefinition::sounds, "sounds")
			.meta("Serialize", PropertySerialize::OPTIONAL)
		.property(&SoundDefinition::soundRanges, "soundRanges")
			.meta("Serialize", PropertySerialize::OPTIONAL)
		;

	SimpleRTTR::registration().type<TransparencyLUTDefinition>()
		.meta("Serialize", ObjectSerialize::ALWAYS)
		.property(&TransparencyLUTDefinition::colors, "colors")
			.meta("Serialize", PropertySerialize::ALWAYS)
		;

	SimpleRTTR::registration().type<ResourceConfigFile>()
		.meta("Serialize", ObjectSerialize::ALWAYS)
		.property(&ResourceConfigFile::soundDefs, "soundDefs")
			.meta("Serialize", PropertySerialize::ALWAYS)
		.property(&ResourceConfigFile::transparencyLUTs, "transparencyLUTs")
			.meta("Serialize", PropertySerialize::ALWAYS)
		;

}

bool loadVanillaResources(Mod* mod, ResourceConfigFile& resourceConfig);

// global function to load 8.1.2 version mods
bool load(Mod* mod)
{
	CompositeFileSystem& fs = mod->getFileSystem();

	ResourceConfigFile resourceConfig;

	Log(LOG_INFO) << "Pre-loading rulesets...";

	// if the mod has a resourceConfigFile, we need to extract the sounds and transparency LUTs
	if(!mod->getInfo().resourceConfigFile.empty())
	{
		std::unique_ptr<FileEntry> resourceConfigFile = fs.getFile(mod->getInfo().resourceConfigFile);
		if (!resourceConfigFile)
		{
			Log(LOG_ERROR) << "Failed to load resource config file '" << mod->getInfo().resourceConfigFile << "' for mod '" << mod->getInfo().id << "'";
			return false;
		}

		// load the resource config file
		YamlFile yamlFile;
		resourceConfig = yamlFile.load<ResourceConfigFile>(resourceConfigFile);
	}

	Log(LOG_INFO) << "Loading vanilla resources...";

	// now we can load the vanilla resources
	if(!loadVanillaResources(mod, resourceConfig))
	{
		return false;
	}

	return true;
}

bool loadVanillaResources(Mod* mod, ResourceConfigFile& resourceConfig)
{
	// Load palettes
	const char *pal[] = { "PAL_GEOSCAPE", "PAL_BASESCAPE", "PAL_GRAPHS", "PAL_UFOPAEDIA", "PAL_BATTLEPEDIA" };

	// Load surfaces


	// Load surface sets
	std::string sets[] = { "BASEBITS.PCK", "INTICON.PCK", "TEXTURE.DAT" };

	// Construct sound sets


	return true;
}


} // namespace ModLoader_8_1_2

} // namespace OpenXcom
