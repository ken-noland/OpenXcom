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
#include <cstdint>
#include <glm/vec4.hpp>

namespace OpenXcom
{

class PackedColor
{
public:
	uint32_t value; // Packed RGBA value in 0xRRGGBBAA format

	PackedColor() : value(0) {}
	PackedColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) { set(r, g, b, a); }
	PackedColor(uint32_t packed) : value(packed) {}
	PackedColor(const glm::vec4& color) { fromVec4(color); }

	void set(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
	{
		value = (r << 24) | (g << 16) | (b << 8) | a;
	}

	uint8_t r() const { return (value >> 24) & 0xFF; }
	uint8_t g() const { return (value >> 16) & 0xFF; }
	uint8_t b() const { return (value >> 8) & 0xFF; }
	uint8_t a() const { return value & 0xFF; }

	glm::vec4 toVec4() const
	{
		return glm::vec4(r() / 255.0f, g() / 255.0f, b() / 255.0f, a() / 255.0f);
	}

	static PackedColor fromVec4(const glm::vec4& color)
	{
		return PackedColor(
			static_cast<uint8_t>(color.r * 255),
			static_cast<uint8_t>(color.g * 255),
			static_cast<uint8_t>(color.b * 255),
			static_cast<uint8_t>(color.a * 255));
	}
};

} // namespace OpenXcom
