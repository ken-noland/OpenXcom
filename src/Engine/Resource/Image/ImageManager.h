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
#include <memory>

namespace OpenXcom
{

class HostImage;
class DeviceImage;
enum class ImageFormat;

class ImageManager
{
public:
	ImageManager() = default;
	virtual ~ImageManager() = default;

	virtual std::unique_ptr<HostImage> createHostImage(ImageFormat format, uint32_t width, uint32_t height) = 0;
	virtual std::unique_ptr<DeviceImage> createDeviceImage(ImageFormat format, uint32_t width, uint32_t height) = 0;
	virtual std::unique_ptr<DeviceImage> createDeviceImage(HostImage& host) = 0;
};

} // namespace OpenXcom
