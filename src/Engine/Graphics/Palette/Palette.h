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

#include "../Types/PackedColor.h"

#include <memory>
#include <string>
#include <initializer_list>

namespace OpenXcom
{

class EngineContext;
class DeviceBuffer;

class Palette
{
private:
	std::string _name;
	std::unique_ptr<DeviceBuffer> _deviceBuffer;

public:
	// create an empty palette
	Palette(EngineContext& context, const std::string& name, std::size_t count);

	// create a palette from an array of colors
	Palette(EngineContext& context, const std::string& name, const PackedColor* data, std::size_t count);

	virtual ~Palette();

	const std::string& getName() const { return _name; };

	DeviceBuffer& getDeviceBuffer() { return *_deviceBuffer; };
};


}
