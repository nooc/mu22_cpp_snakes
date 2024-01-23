#pragma once

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/fileconf.h>

#include "engine.h"

namespace snakes
{
	enum {
		BTN_CONNECT = 1000,
		BTN_HOST,
		BTN_HISTORY
	};

	class MainFrame : public wxFrame
	{
		private:
			wxFileConfig m_cfg;
			Engine* m_game;

			wxPanel* m_gameView;
			wxListView* m_scorelist;
			wxButton* m_historyButton;
			wxButton* m_connectButton;
			wxButton* m_hotsButton;
			wxStaticText* m_connectionTexrt;

			void PaintGame(wxPaintEvent& event);
			void ConnectToRemote(wxCommandEvent& event);
			void StartAsHost(wxCommandEvent& event);
			void ShowHistory(wxCommandEvent& event);
			void ProcessSocket(wxSocketEvent& event);

			void _EnableConnectButtons(bool ebabled);
			void _CreateGame(EngineType mode);
		public:

			MainFrame( const wxString& title = wxEmptyString,
				const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ),
				long style = wxCAPTION|wxCLOSE_BOX|wxSTAY_ON_TOP|wxTAB_TRAVERSAL );

			~MainFrame();
	};

}
