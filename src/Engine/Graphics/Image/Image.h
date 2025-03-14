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
#include <string>
#include <glm/vec2.hpp>

namespace OpenXcom
{

class DeviceBuffer;

enum class ImageFormat
{
	R8,       // Grayscale or palette index
	R8G8B8,   // RGB
	R8G8B8A8, // RGBA
	UNKNOWN
};

constexpr int bytesPerPixel(ImageFormat format)
{
	switch (format)
	{
	case ImageFormat::R8:
		return 1;
	case ImageFormat::R8G8B8:
		return 3;
	case ImageFormat::R8G8B8A8:
		return 4;
	default:
		return 0;
	}
}


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
	std::string _name;

public:
	HostImage(ImageType type, const std::string& name) : _type(type), _name(name) {};
	virtual ~HostImage() = default;

	const std::string& getName() const { return _name; }

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
	std::string _name;

public:
	DeviceImage(ImageType type, const std::string& name) : _type(type), _name(name) {};
	virtual ~DeviceImage() = default;

	const std::string& name() const { return _name; }

	// copy the host image to this device image
	virtual void copyFrom(HostImage& hostImage) = 0;

	// copy this device image to the host image
	virtual void copyTo(HostImage& hostImage) = 0;

	// device image data for an contains the extents of the image stored on the GPU
	virtual const DeviceBuffer& getDeviceImageData() const = 0;

	virtual ImageFormat getFormat() const = 0;
	virtual glm::ivec2 getExtent() const = 0;
	virtual uint32_t getWidth() const = 0;
	virtual uint32_t getHeight() const = 0;

	ImageType getType() const { return _type; }
};

} // namespace OpenXcom
