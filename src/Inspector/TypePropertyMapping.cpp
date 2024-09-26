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
#include "TypePropertyMapping.h"

namespace OpenXcom
{

template <typename Type>
struct PropertyTypeHelper
{
	static wxWindow* createProperty(wxWindow* parent, entt::handle entityHandle, const SimpleRTTR::Type& componentType, const SimpleRTTR::Property& componentProperty)
	{
		return new wxStaticText(parent, wxID_ANY, "Unable to create property sheet for type \"" + componentProperty.type().name() + "\"");
	}
};

template <>
struct PropertyTypeHelper<int>
{
	static wxWindow* createProperty(wxWindow* parent, entt::handle entityHandle, const SimpleRTTR::Type& componentType, const SimpleRTTR::Property& componentProperty)
	{
		//example
		GetComponentFunc getComponentFunc = componentType.meta().get(GetComponentFuncName).value().get_as<GetComponentFunc>();
		if (getComponentFunc == nullptr)
		{
			std::string message = "Unable to create integer property sheet for component type \"" + componentType.name() + "\"";
			return new wxStaticText(parent, wxID_ANY, message);
		}

		int initialValue = componentProperty.get<int>(getComponentFunc(entityHandle));
		wxTextCtrl* textCtrl = new wxTextCtrl(parent, wxID_ANY, std::to_string(initialValue), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxNO_BORDER);
		textCtrl->SetValidator(*createValidator());

		// Bind the event to update the entity's property when the text is changed
		textCtrl->Bind(wxEVT_TEXT_ENTER, [entityHandle, componentType, componentProperty, textCtrl](wxCommandEvent& event)
		{
			GetComponentFunc getComponentFunc = componentType.meta().get(GetComponentFuncName).value().get_as<GetComponentFunc>();
			if (getComponentFunc != nullptr)
			{
				wxString valueStr = textCtrl->GetValue();
				long newValue;
				if (valueStr.ToLong(&newValue)) // Validate the conversion
				{
					componentProperty.set(getComponentFunc(entityHandle), static_cast<int>(newValue));
				}
			}
		});

		return textCtrl;
	}

	static wxValidator* createValidator()
	{
		//example
		return new wxIntegerValidator<int>();
	}
};


template <>
struct PropertyTypeHelper<std::string>
{
	static wxWindow* createProperty(wxWindow* parent, entt::handle entityHandle, const SimpleRTTR::Type& componentType, const SimpleRTTR::Property& componentProperty)
	{
		// example
		GetComponentFunc getComponentFunc = componentType.meta().get(GetComponentFuncName).value().get_as<GetComponentFunc>();
		if (getComponentFunc == nullptr) {
			std::string message = "Unable to create integer property sheet for component type \"" + componentType.name() + "\"";
			return new wxStaticText(parent, wxID_ANY, message);
		}

		std::string initialValue = componentProperty.get<std::string>(getComponentFunc(entityHandle));
		wxTextCtrl* textCtrl = new wxTextCtrl(parent, wxID_ANY, initialValue, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxNO_BORDER);

		// Bind the event to update the entity's property when the text is changed
		textCtrl->Bind(wxEVT_TEXT_ENTER, [entityHandle, componentType, componentProperty, textCtrl](wxCommandEvent& event) {
			GetComponentFunc getComponentFunc = componentType.meta().get(GetComponentFuncName).value().get_as<GetComponentFunc>();
			if (getComponentFunc != nullptr) {
				wxString valueStr = textCtrl->GetValue();
				componentProperty.set(getComponentFunc(entityHandle), std::string(valueStr));
			}
		});

		return textCtrl;
	}
};


template <>
struct PropertyTypeHelper<bool>
{
	static wxWindow* createProperty(wxWindow* parent, entt::handle entityHandle, const SimpleRTTR::Type& componentType, const SimpleRTTR::Property& componentProperty)
	{
		// Retrieve the component instance from the entity
		GetComponentFunc getComponentFunc = componentType.meta().get(GetComponentFuncName).value().get_as<GetComponentFunc>();
		if (getComponentFunc == nullptr)
		{
			std::string message = "Unable to create boolean property sheet for component type \"" + componentType.name() + "\"";
			return new wxStaticText(parent, wxID_ANY, message);
		}

		// Get the initial value of the boolean property
		bool initialValue = componentProperty.get<bool>(getComponentFunc(entityHandle));

		// Create a checkbox with the initial value
		wxCheckBox* checkBox = new wxCheckBox(parent, wxID_ANY, wxEmptyString);
		checkBox->SetValue(initialValue);

		// Bind the checkbox to the property
		checkBox->Bind(wxEVT_CHECKBOX, [entityHandle, componentType, componentProperty, checkBox](wxCommandEvent& event)
		{
			GetComponentFunc getComponentFunc = componentType.meta().get(GetComponentFuncName).value().get_as<GetComponentFunc>();
			if (getComponentFunc != nullptr)
			{
				bool newValue = checkBox->GetValue();
				componentProperty.set(getComponentFunc(entityHandle), newValue);
			}
		});

		return checkBox;
	}
};


TypePropertyMapping::TypePropertyMapping()
{
	createTypeMapping();
}

TypePropertyMapping::~TypePropertyMapping()
{
}

void TypePropertyMapping::createTypeMapping()
{
	_typeMapping[SimpleRTTR::types().get_type<int>().type_index()] = PropertyTypeHelper<int>::createProperty;
	_typeMapping[SimpleRTTR::types().get_type<bool>().type_index()] = PropertyTypeHelper<bool>::createProperty;
	_typeMapping[SimpleRTTR::types().get_type<std::string>().type_index()] = PropertyTypeHelper<std::string>::createProperty;
}

wxWindow* TypePropertyMapping::createProperty(wxWindow* parent, entt::handle entityHandle, const SimpleRTTR::Type& componentType, const SimpleRTTR::Property& componentProperty)
{
	SimpleRTTR::Type propertyType = componentProperty.type();
	TypeMap::iterator it = _typeMapping.find(propertyType.type_index());

	if (it != _typeMapping.end())
	{
		// Use the registered type handler
		return it->second(parent, entityHandle, componentType, componentProperty);
	}
	else
	{
		// Fallback if no handler is registered for this type
		std::string message = "Unable to create property sheet for type \"" + propertyType.name() + "\"";
		return new wxStaticText(parent, wxID_ANY, message);
	}
}

void TypePropertyMapping::updateProperty(wxPGProperty* property)
{
}

} // namespace OpenXcom
