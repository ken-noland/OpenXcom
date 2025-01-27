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
#include <cstddef>

namespace OpenXcom
{

enum class BufferUsage
{
	Vertex,
	Index,
	Uniform,
	Storage,
};

class HostBuffer
{
protected:
	BufferUsage _usage;

public:
	HostBuffer(BufferUsage usage) : _usage(usage) {}
	virtual ~HostBuffer() = default;

	const BufferUsage getUsage() const { return _usage; }

	virtual void copyTo(const void* data, std::size_t offset, std::size_t size) = 0;
};

class DeviceBuffer
{
protected:
	BufferUsage _usage;

public:
	DeviceBuffer(BufferUsage usage) : _usage(usage) {}
	virtual ~DeviceBuffer() = default;

	const BufferUsage getUsage() const { return _usage; }
};

} // namespace OpenXcom
