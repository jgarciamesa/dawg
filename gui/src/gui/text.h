#pragma once
#include <wx/wx.h>
#include "page.h"

class dawgText
{
public:
	dawgText(dawgPage* page, const wxString& key, const std::string& def = "");
	std::string GetValue();
	std::string GetTextValue();
	~dawgText(void);

private:
	wxStaticText *label;
	wxTextCtrl   *textctrl;
	wxString     wxDef;
};
