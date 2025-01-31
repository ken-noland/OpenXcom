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
#include "Palette.h"

#include "../../EngineContext.h"
#include "../../Resource/ResourceSystem.h"
#include "../BufferManager.h"

namespace OpenXcom
{

Palette::Palette(EngineContext& context, const std::string& name, const PackedColor* data, std::size_t count)
{
	_deviceBuffer = context.getResourceSystem().getBufferManager().createDeviceBuffer<PackedColor>(data, count, BufferUsage::Storage);
}

Palette::Palette(EngineContext& context, const std::string& name, std::initializer_list<PackedColor> data)
	: Palette(context, name, static_cast<const PackedColor*>(data.begin()), data.size())
{
}

Palette::~Palette()
{
}

} // namespace OpenXcom
