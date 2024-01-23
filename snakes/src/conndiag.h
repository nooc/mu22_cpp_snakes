#pragma once

#include <wx/wx.h>
#include <wx/confbase.h>

#include "engine.h"

namespace snakes
{
	class ConnectionDialog : public wxDialog
	{
		private:

			wxTextCtrl* m_textCtrl1;
			wxButton* m_button3;
			wxButton* m_button4;

		public:

			ConnectionDialog(wxWindow* parent, EngineType mode, wxConfigBase& config);

			~ConnectionDialog();

			wxString GetValue();

	};

} // namespace snakes

