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

// Run time type declarations for GLM types
#include <simplerttr.h>
#include <glm/glm.hpp>

// Run time type information
SIMPLERTTR
{
	SimpleRTTR::registration().type<glm::vec2>()
		.property(&glm::vec2::x, "x")
		.property(&glm::vec2::y, "y");

	SimpleRTTR::registration().type<glm::ivec2>()
		.property(&glm::ivec2::x, "x")
		.property(&glm::ivec2::y, "y");

	SimpleRTTR::registration().type<glm::vec3>()
		.property(&glm::vec3::x, "x")
		.property(&glm::vec3::y, "y")
		.property(&glm::vec3::z, "z");

	SimpleRTTR::registration().type<glm::mat4>()
		.meta("name", "glm::mat4");
}

namespace OpenXcom
{

//hack to get this to link
int FORCE_LINK_RTTRGLM = 0;

} // namespace OpenXcom
