#pragma once
#include <wx/wx.h>
#include <wx/string.h>
#include "page.h"

class dawgText
{
public:
	dawgText(dawgPage* page, const wxString& key, const std::string& def = "");
	~dawgText(void);

private:
	wxStaticText *label;
	wxTextCtrl   *textctrl;
};
