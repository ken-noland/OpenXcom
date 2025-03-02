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

#include "PlatformWindow.h"
#include "../Resource/ResourceManager.h"

namespace OpenXcom
{

class PlatformWindowSystem : public ResourceManager<PlatformWindow>
{
public:
	PlatformWindowSystem() = default;
	virtual ~PlatformWindowSystem() = default;

	virtual OwningHandle<PlatformWindow> create(const std::string& title, int width, int height)
	{
		std::unique_ptr<PlatformWindow> window(new PlatformWindow(title, width, height));
		return add(std::move(window));
	}
};

} // namespace OpenXcom
