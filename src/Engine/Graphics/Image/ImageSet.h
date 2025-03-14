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


namespace OpenXcom
{

class ImageSet
{
protected:
	std::string _name;
	std::vector<ResourceHandle<DeviceImage>> _images;

public:
	ImageSet() = default;
	ImageSet(const std::string& name) : _name(name) { }

	// Copy constructor
	ImageSet(const ImageSet& other) = default;

	// Move constructor
	ImageSet(ImageSet&& other) noexcept
	{
		_name = std::move(other._name);
		_images = std::move(other._images);
	};

	~ImageSet() = default;

	ImageSet& operator=(ImageSet&& other) noexcept
	{
		_name = std::move(other._name);
		_images = std::move(other._images);
		return *this;
	}

	const std::string& name() const
	{
		return _name;
	}

	void setName(const std::string& name)
	{
		_name = name;
	}

	const std::vector<ResourceHandle<DeviceImage>>& images() const
	{
		return _images;
	}

	void add(ResourceHandle<DeviceImage> image)
	{
		_images.push_back(image);
	}
};

} // namespace OpenXcom
