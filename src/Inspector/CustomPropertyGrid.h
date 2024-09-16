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

namespace OpenXcom
{

class CustomSplitterWindow : public wxSplitterWindow
{
public:
	CustomSplitterWindow(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
						 const wxSize& size = wxDefaultSize, long style = wxSP_3D);

protected:
	void DrawSashTracker(int x, int y) override;
	//	void OnSashPositionChanging(int newSashPosition) override;
};


class CustomPropertyGrid : public wxScrolledWindow
{
public:
	CustomPropertyGrid(wxWindow* parent);
	~CustomPropertyGrid();

	void Add(const wxString& name, std::function<wxWindow*(wxWindow*)> createValueCtrl);

	void Clear();

private:
	CustomSplitterWindow* _splitterWindow;
	wxPanel* _leftPanel;
	wxPanel* _rightPanel;

	wxBoxSizer* _leftSizer;
	wxBoxSizer* _rightSizer;

	int _totalContentHeight;
};

} // namespace OpenXcom
