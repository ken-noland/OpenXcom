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
#include "../../../Resource/Palette/PaletteManager.h"
#include "VulkanPalette.h"

namespace OpenXcom
{

class VulkanContext;

class VulkanPaletteManager : public PaletteManager
{
public:
	VulkanPaletteManager(VulkanContext& context);
	virtual ~VulkanPaletteManager();
};

} // namespace OpenXcom
