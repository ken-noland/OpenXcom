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

#include "WxInclude.h"
#include <simplerttr.h>
#include "../Entity/Common/RTTR.h"

namespace OpenXcom
{

class TypePropertyMapping
{
protected:
	void createTypeMapping();

	using TypeMap = std::unordered_map<std::type_index, std::function<wxWindow*(wxWindow*, entt::handle, const SimpleRTTR::Type&, const SimpleRTTR::Property&)>>;
	TypeMap _typeMapping;

public:
	TypePropertyMapping();
	~TypePropertyMapping();

	wxWindow* createProperty(wxWindow* parent, entt::handle entityHandle, const SimpleRTTR::Type& componentType, const SimpleRTTR::Property& componentProperty);
	void updateProperty(wxPGProperty* property);
};

} // namespace OpenXcom
