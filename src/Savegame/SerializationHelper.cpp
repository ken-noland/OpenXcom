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
#include "SerializationHelper.h"
#include <assert.h>
#include <sstream>
#include <cfloat>

#include "../Engine/Logger.h"
#include "../Engine/CrossPlatform.h"

namespace OpenXcom
{

int unserializeInt(uint8_t **buffer, uint8_t sizeKey)
{
	/* The C spec explicitly requires *(Type*) pointer accesses to be
	 * sizeof(Type) aligned, which is not guaranteed by the UInt8** buffer
	 * passed in here.
	 * memcpy() is explicitly designed to cope with any address alignment, so
	 * use that to avoid undefined behaviour */
	int ret = 0;
	switch(sizeKey)
	{
	case 1:
		uint8_t tmp;
		memcpy(&tmp, *buffer, sizeof(tmp));
		ret = tmp;
		break;
	case 2:
	{
		int16_t tmp;
		memcpy(&tmp, *buffer, sizeof(tmp));
		ret = tmp;
		break;
	}
	case 4:
	{
		uint32_t tmp;
		memcpy(&tmp, *buffer, sizeof(tmp));
		ret = tmp;
		break;
	}
	default:
	{
		// XXX: if this unreachable block gets hit, check to see
		//   that sizeKey is being properly initialized
		#ifdef NDEBUG
			Log(LOG_WARNING) << "unserializeInt has invalid sizeKey of " << sizeKey
				<< " .. this can mean deserialization data is ill-formed";
		#else
			CrossPlatform::unreachable();
		#endif
	}
	}

	*buffer += sizeKey;

	return ret;
}

void serializeInt(uint8_t **buffer, uint8_t sizeKey, int value)
{
	/* The C spec explicitly requires *(Type*) pointer accesses to be
	 * sizeof(Type) aligned, which is not guaranteed by the UInt8** buffer
	 * passed in here.
	 * memcpy() is explicitly designed to cope with any address alignment, so
	 * use that to avoid undefined behaviour */
	switch(sizeKey)
	{
	case 1:
	{
		uint8_t u8Value = value;
		assert(value < 256);
		memcpy(*buffer, &u8Value, sizeof(uint8_t));
		break;
	}
	case 2:
	{
		int16_t s16Value = value;
		assert(value < 65536);
		memcpy(*buffer, &s16Value, sizeof(int16_t));
		break;
	}
	case 4:
	{
		uint32_t u32Value = value;
		memcpy(*buffer, &u32Value, sizeof(uint32_t));
		break;
	}
	default:
	{
		// XXX: if this unreachable block gets hit, check to see
		//   that sizeKey is being properly initialized
		#ifdef NDEBUG
			Log(LOG_WARNING) << "serializeInt has invalid sizeKey of " << sizeKey
				<< " .. this can mean serialization data is ill-formed";
		#else
			CrossPlatform::unreachable();
		#endif
	}
	}

	*buffer += sizeKey;
}

std::string serializeDouble(double value)
{
	std::ostringstream stream;
	stream.precision(DBL_DIG + 2);
	stream << value;
	return stream.str();
}

}
