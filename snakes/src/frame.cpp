#include <wx/wxprec.h>

#include <random>

#include "frame.h"
#include "histdiag.h"
#include "conndiag.h"

using namespace snakes;

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size, long style)
	: wxFrame(0L, wxID_ANY, title, pos, size, style),
	m_cfg("snakes", wxEmptyString, "snakes.cfg", wxEmptyString, wxCONFIG_USE_LOCAL_FILE)
{
	SetSizeHints(wxDefaultSize, wxDefaultSize);
	SetMaxSize(wxSize(600, 1080));

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	// Size of game area
	int boardDim = GAME_CELL_SIZE * GAME_LINE_SIZE;

	m_gameView = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(boardDim, boardDim), wxBORDER_NONE);
	m_gameView->SetBackgroundColour(wxColour(0, 0, 0));
	m_gameView->SetMinSize(wxSize(boardDim, boardDim));
	m_gameView->SetMaxSize(wxSize(boardDim, boardDim));

	bSizer1->Add(m_gameView, 1, 0, 5);

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Scores")), wxHORIZONTAL);

	m_scorelist = new wxListView(sbSizer2->GetStaticBox());
	sbSizer2->Add(m_scorelist, 1, wxALL, 5);

	m_historyButton = new wxButton(sbSizer2->GetStaticBox(), BTN_HISTORY, wxT("History"), wxDefaultPosition, wxDefaultSize, 0);
	sbSizer2->Add(m_historyButton, 0, wxALL, 5);


	bSizer2->Add(sbSizer2, 1, wxALL | wxEXPAND, 5);

	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Game")), wxVERTICAL);

	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer(wxHORIZONTAL);

	m_connectButton = new wxButton(sbSizer1->GetStaticBox(), BTN_CONNECT, wxT("Connect"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer5->Add(m_connectButton, 1, wxALL, 5);

	m_hotsButton = new wxButton(sbSizer1->GetStaticBox(), BTN_HOST, wxT("Host"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer5->Add(m_hotsButton, 1, wxALL, 5);


	sbSizer1->Add(bSizer5, 1, wxEXPAND, 5);

	m_connectionTexrt = new wxStaticText(sbSizer1->GetStaticBox(), wxID_ANY, wxT("Disconnected"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
	m_connectionTexrt->Wrap(-1);
	sbSizer1->Add(m_connectionTexrt, 0, wxALL | wxEXPAND, 5);


	bSizer2->Add(sbSizer1, 0, wxALL | wxEXPAND, 5);


	bSizer1->Add(bSizer2, 0, wxEXPAND, 5);


	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);

	this->Centre(wxBOTH);

	// Connect Events
	m_gameView->Connect(wxEVT_PAINT, wxPaintEventHandler(MainFrame::PaintGame), NULL, this);
	m_connectButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::ConnectToRemote), NULL, this);
	m_hotsButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::StartAsHost), NULL, this);
	m_historyButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::ShowHistory), NULL, this);
}

MainFrame::~MainFrame()
{
	// Disconnect Events
	m_gameView->Disconnect(wxEVT_PAINT, wxPaintEventHandler(MainFrame::PaintGame), NULL, this);
	m_connectButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::ConnectToRemote), NULL, this);
	m_hotsButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::StartAsHost), NULL, this);
	m_historyButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::ShowHistory), NULL, this);

}

void MainFrame::PaintGame(wxPaintEvent& event)
{
	wxPaintDC dc(m_gameView);
	dc.SetPen(*wxTRANSPARENT_PEN);

	int size = GAME_LINE_SIZE * GAME_CELL_SIZE;

	for (int y = 0; y < size; y += GAME_CELL_SIZE)
	{
		for (int x = 0; x < size; x += GAME_CELL_SIZE)
		{
			dc.SetBrush(*wxGREEN_BRUSH);
			dc.DrawRectangle(x, y, GAME_CELL_SIZE, GAME_CELL_SIZE);
		}
	}
}

void MainFrame::ConnectToRemote(wxCommandEvent& event)
{
	ConnectionDialog dlg(this, CMODE_CLIENT, m_cfg);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxMessageBox(dlg.GetValue(), wxT("Connect"), wxICON_INFORMATION);
	}
}

void MainFrame::StartAsHost(wxCommandEvent& event)
{
	ConnectionDialog dlg(this, CMODE_HOST, m_cfg);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxMessageBox(dlg.GetValue(), wxT("Connect"), wxICON_INFORMATION);
	}
}

void MainFrame::ShowHistory(wxCommandEvent& event)
{
	HistoryDialog dlg(this);
	dlg.ShowModal();
}
