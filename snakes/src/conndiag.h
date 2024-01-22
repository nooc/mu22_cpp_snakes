#pragma once

#include <wx/wx.h>
#include <wx/confbase.h>

namespace snakes
{
	enum ConnectionMode
	{
		CMODE_CLIENT,
		CMODE_HOST
	};

	class ConnectionDialog : public wxDialog
	{
		private:

			wxTextCtrl* m_textCtrl1;
			wxButton* m_button3;
			wxButton* m_button4;

		public:

			ConnectionDialog(wxWindow* parent, ConnectionMode mode, wxConfigBase& config);

			~ConnectionDialog();

			wxString GetValue();

	};

} // namespace snakes

