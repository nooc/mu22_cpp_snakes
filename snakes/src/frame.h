///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.0.0-0-g0efcecf)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/fileconf.h>

namespace snakes
{
	// Size of a game block
	#define GAME_CELL_SIZE 10
	// Number of blocks making up a full horizontal or vertical line.
	#define GAME_LINE_SIZE 60

	enum {
		BTN_CONNECT = 1000,
		BTN_HOST,
		BTN_HISTORY
	};

	class MainFrame : public wxFrame
	{
		private:
			wxFileConfig m_cfg;

		protected:
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


		public:

			MainFrame( const wxString& title = wxEmptyString,
				const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ),
				long style = wxCAPTION|wxCLOSE_BOX|wxSTAY_ON_TOP|wxTAB_TRAVERSAL );

			~MainFrame();

	};

} // namespace snakes

