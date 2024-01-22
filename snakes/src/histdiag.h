#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/listbox.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/dialog.h>


namespace snakes
{
	class HistoryDialog : public wxDialog
	{
	private:

	protected:
		wxListBox* m_listBox1;
		wxButton* m_button6;

		// Virtual event handlers, override them in your derived class
		//virtual void handleClose(wxCommandEvent& event) { event.Skip(); }


	public:

		HistoryDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Past Games"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);

		~HistoryDialog();

	};

}
