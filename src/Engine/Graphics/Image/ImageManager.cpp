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
#include "ImageManager.h"

#include "Image.h"

namespace OpenXcom
{

bool ImageManager::exists(const ResourceHandle<HostImage>& handle) const
{
	return _hostImageManager.exists(handle);
}

bool ImageManager::exists(const ResourceHandle<DeviceImage>& handle) const
{
	return _deviceImageManager.exists(handle);
}

HostImage& ImageManager::get(const ResourceHandle<HostImage>& handle)
{
	return _hostImageManager.get(handle);
}

DeviceImage& ImageManager::get(const ResourceHandle<DeviceImage>& handle)
{
	return _deviceImageManager.get(handle);
}


} // namespace OpenXcom
