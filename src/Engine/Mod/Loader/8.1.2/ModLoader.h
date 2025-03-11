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
#include <string>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace OpenXcom
{

class Mod;

namespace ModLoader_8_1_2
{

// ---
// Helper structs to do the loading of the 8.1.2 version mods.
// ---

// Sound definition
struct SoundDefinition
{
	std::string type;
	std::string file;
	std::vector<int> sounds;
	std::vector<std::pair<int, int>> soundRanges;
};

// Transparency LUT definition
struct TransparencyLUTDefinition
{
	std::vector<glm::ivec4> colors;
};

// The resource config file is a yaml file that contains definitions for sounds and transparency LUTs. The old way of
// loading mods used these values to extract the sound files and the transparency LUTs prior to the loading of the
// vanilla resources. In the case of soundDefs, it uses the definition to know which sound files to load. In the case
// of the transparencyLUTs, it uses the look up table to pre-compute the transparency values for the palette.
struct ResourceConfigFile
{
	std::vector<SoundDefinition> soundDefs;
	std::vector<TransparencyLUTDefinition> transparencyLUTs;
};


// global function to load 8.1.2 version mods
bool load(Mod* mod);

} // namespace ModLoader_8_1_2

} // namespace OpenXcom
