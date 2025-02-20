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

#define OPENXCOM_VERSION_MAJOR 0
#define OPENXCOM_VERSION_MINOR 0
#define OPENXCOM_VERSION_PATCH 1

#define OPENXCOM_VERSION_STRINGIFY(x) #x
#define OPENXCOM_VERSION_XSTRINGIFY(x) OPENXCOM_VERSION_STRINGIFY(x)

#define OPENXCOM_VERSION_ENGINE "OX-L"
#define OPENXCOM_VERSION_LONG OPENXCOM_VERSION_XSTRINGIFY(OPENXCOM_VERSION_MAJOR) "." OPENXCOM_VERSION_XSTRINGIFY(OPENXCOM_VERSION_MINOR) "." OPENXCOM_VERSION_XSTRINGIFY(OPENXCOM_VERSION_PATCH) ".0"
#define OPENXCOM_VERSION_SHORT "Lua " OPENXCOM_VERSION_LONG
#define OPENXCOM_VERSION_NUMBER OPENXCOM_VERSION_MAJOR, OPENXCOM_VERSION_MINOR, OPENXCOM_VERSION_PATCH, 0

#ifndef OPENXCOM_VERSION_GIT
#define OPENXCOM_VERSION_GIT " (v2024-09-05)"
#endif
