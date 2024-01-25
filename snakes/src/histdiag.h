#pragma once

#include <wx/wx.h>

#include "history.h"

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

		HistoryDialog(wxWindow* parent, HistoryManager* history);
	};

}
