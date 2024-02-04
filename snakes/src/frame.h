#pragma once

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/fileconf.h>

#include "engine.h"
#include "history.h"

namespace snakes
{
	enum {
		BTN_CONNECT = 1000,
		BTN_HOST,
		BTN_HISTORY,
		BTN_DISCONNECT
	};

	class MainFrame : public wxFrame
	{
		private:
			wxFileConfig m_cfg;
			Engine* m_game;
			HistoryManager m_hist;

			wxPanel* m_gameView;
			wxListBox* m_scorelist;
			wxButton* m_historyButton;
			wxButton* m_connectButton;
			wxButton* m_hotsButton;
			wxButton* m_disconnectButton;
			wxTextCtrl* m_nickName;
			wxListView* m_playerList;

			void PaintGame(wxPaintEvent& event);
			void ConnectToRemote(wxCommandEvent& event);
			void StartAsHost(wxCommandEvent& event);
			void DisconnectGame(wxCommandEvent& event);
			void ShowHistory(wxCommandEvent& event);
			void ProcessSocket(wxSocketEvent& event);
			void ProcessKeyInput(wxKeyEvent& event);
			void ProcessTick(wxTimerEvent& event);
			void OnEngineEvent(EngineEvent& event);

			void AskReady();
			void _EnableConnectButtons(bool ebabled);
			void _CreateGame(EngineType mode);
			void _Terminate();
		public:

			MainFrame( const wxString& title = wxEmptyString,
				const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ),
				long style = wxCAPTION|wxCLOSE_BOX|wxSTAY_ON_TOP|wxTAB_TRAVERSAL );

			~MainFrame();
	};

}
