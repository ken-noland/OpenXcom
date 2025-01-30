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
#include <glm/vec2.hpp>

namespace OpenXcom
{

enum class ImageFormat
{
	UNKNOWN,
	RGBA8,
};

enum class ImageType
{
	Texture,
	RenderTarget,
	Surface
};

class HostImage
{
protected:
	ImageType _type;

public:
	HostImage(ImageType type) : _type(type) {};
	virtual ~HostImage() = default;

	virtual ImageFormat getFormat() const = 0;
	virtual glm::ivec2 getExtent() const = 0;
	virtual uint32_t getWidth() const = 0;
	virtual uint32_t getHeight() const = 0;

	ImageType getType() const { return _type; }

	virtual void* map() = 0;
	virtual void unmap() = 0;
};

class DeviceImage
{
protected:
	ImageType _type;

public:
	DeviceImage(ImageType type) : _type(type) {};
	virtual ~DeviceImage() = default;

	// copy the host image to this device image
	virtual void copyFrom(HostImage& hostImage) = 0;

	// copy this device image to the host image
	virtual void copyTo(HostImage& hostImage) = 0;


	virtual ImageFormat getFormat() const = 0;
	virtual glm::ivec2 getExtent() const = 0;
	virtual uint32_t getWidth() const = 0;
	virtual uint32_t getHeight() const = 0;

	ImageType getType() const { return _type; }
};

} // namespace OpenXcom
