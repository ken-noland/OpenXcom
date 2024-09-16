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
#include "CustomPropertyGrid.h"

namespace OpenXcom
{

CustomSplitterWindow::CustomSplitterWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	: wxSplitterWindow(parent, id, pos, size, style)
{

}

void CustomSplitterWindow::DrawSashTracker(int x, int y)
{
	int w, h;
	wxWindow* parent = GetParent();
	//use the parent client size
	parent->GetClientSize(&w, &h);

	int x1, y1;
	int x2, y2;

	if (m_splitMode == wxSPLIT_VERTICAL)
	{
		x1 = x2 = wxClip(x, 0, w) + m_sashTrackerPen->GetWidth() / 2;
		y1 = 2;
		y2 = h - 2;
	}
	else
	{
		y1 = y2 = wxClip(y, 0, h) + m_sashTrackerPen->GetWidth() / 2;
		x1 = 2;
		x2 = w - 2;
	}

#if defined(__WXGTK3__)
	wxClientDC dc(this);

	// In the ports with wxGraphicsContext-based wxDC, such as wxGTK3 or wxOSX,
	// wxINVERT only works for inverting the background when using white
	// foreground (note that this code is not used anyhow for __WXMAC__ due to
	// always using live-resizing there, see IsLive()).
	dc.SetPen(*wxWHITE_PEN);
#else
	// We need to use wxScreenDC and not wxClientDC at least for wxMSW where
	// drawing in this window itself would be hidden by its children windows,
	// that cover it almost entirely.
	wxScreenDC dc;
	parent->ClientToScreen(&x1, &y1);
	parent->ClientToScreen(&x2, &y2);

	dc.SetPen(*m_sashTrackerPen);
#endif

	dc.SetLogicalFunction(wxINVERT);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.DrawLine(x1, y1, x2, y2);
}


CustomPropertyGrid::CustomPropertyGrid(wxWindow* parent)
	: wxScrolledWindow(parent, wxID_ANY), _totalContentHeight(0)
{
	SetScrollRate(5, 5);

	// Set the background color to match wxSYS_COLOUR_BTNFACE
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

	// Create the splitter window
	_splitterWindow = new CustomSplitterWindow(this, wxID_ANY);

	// Set the sizer for scrolledWindow
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(_splitterWindow, 1, wxEXPAND);
	SetSizer(sizer);

	// Create the left and right panes
	_leftPanel = new wxPanel(_splitterWindow, wxID_ANY);
	_rightPanel = new wxPanel(_splitterWindow, wxID_ANY);

	// Initialize the splitter
	_splitterWindow->SplitVertically(_leftPanel, _rightPanel);
	_splitterWindow->SetSashGravity(0.3);

	// Sizers for left and right panes
	_leftSizer = new wxBoxSizer(wxVERTICAL);
	_leftPanel->SetSizer(_leftSizer);

	_rightSizer = new wxBoxSizer(wxVERTICAL);
	_rightPanel->SetSizer(_rightSizer);
}

CustomPropertyGrid::~CustomPropertyGrid()
{
	// The controls will be destroyed automatically by wxWidgets
}

void CustomPropertyGrid::Add(const wxString& name, std::function<wxWindow*(wxWindow*)> createValueCtrl)
{
	// Left panel: property name
	wxStaticText* nameCtrl = new wxStaticText(_leftPanel, wxID_ANY, name);
	_leftSizer->Add(nameCtrl, 0, wxEXPAND | wxALL, 5);

	// Right panel: property value
	wxWindow* valueCtrl = createValueCtrl(_rightPanel);
	_rightSizer->Add(valueCtrl, 0, wxEXPAND | wxALL, 5);

    // Get the preferred height
	int rowHeight = std::max(nameCtrl->GetBestSize().GetHeight(), valueCtrl->GetBestSize().GetHeight());

	// Set minimum sizes
	nameCtrl->SetMinSize(wxSize(-1, rowHeight));
	valueCtrl->SetMinSize(wxSize(-1, rowHeight));

	_totalContentHeight += rowHeight;

	// Update layouts
	_leftPanel->Layout();
	_rightPanel->Layout();

	// Update virtual size
	SetVirtualSize(-1, _totalContentHeight);
}

void CustomPropertyGrid::Clear()
{
	_leftSizer->Clear(true);
	_rightSizer->Clear(true);
	_totalContentHeight = 0;

	// Update virtual size
	SetVirtualSize(-1, 0);

	_leftPanel->Layout();
	_rightPanel->Layout();
}

} // namespace OpenXcom
