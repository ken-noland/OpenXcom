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
#include "YamlAssert.h"
#include "Logger.h"
namespace OpenXcom
{

YamlAssertHandler::~YamlAssertHandler()
{
	if (_active)
	{
		Log(LOG_ERROR) << "Assertion failed: " << _expr
				  << ", file " << _file << ", line " << _line << ". ";
		if (!_ss.str().empty())
			Log(LOG_ERROR) << _ss.str();
		Log(LOG_ERROR) << std::endl;
		std::abort();
	}
}

} // namespace OpenXcom
