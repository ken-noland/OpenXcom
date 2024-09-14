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

// we need to wrap all wxWidgets includes in this file to avoid conflicts with other systems
#pragma push_macro("Log")
#undef Log

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/treectrl.h>
#include <wx/timer.h>
#include <wx/splitter.h>
#include <wx/srchctrl.h>
#include <wx/propgrid/propgrid.h>

#pragma pop_macro("Log")
