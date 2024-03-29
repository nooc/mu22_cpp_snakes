#include <wx/wxprec.h>
#include <wx/dcbuffer.h>
#include <wx/imaglist.h>

#include "frame.h"
#include "histdiag.h"
#include "conndiag.h"
#include "nick_gen.h"

using namespace snakes;

static const wxColor s_foodColor = wxColor(0xFF1493);
static const wxPen* s_pens[] = { wxBLACK_PEN, wxGREEN_PEN, wxBLUE_PEN, wxYELLOW_PEN, wxRED_PEN };
static const wxBrush* s_brushes[] = { wxBLACK_BRUSH, wxGREEN_BRUSH, wxBLUE_BRUSH, wxYELLOW_BRUSH, wxRED_BRUSH };

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size, long style)
	: wxFrame(0L, wxID_ANY, title, pos, size, style),
	m_cfg("snakes", wxEmptyString, "snakes.cfg", wxEmptyString, wxCONFIG_USE_LOCAL_FILE),
	m_game(NULL), m_hist(wxT("history.txt"))
{
	SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	// Size of game canvas
	int boardDim = GAME_CELL_SIZE * GAME_LINE_SIZE;

	m_gameView = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(boardDim, boardDim), wxBORDER_NONE);
	m_gameView->SetBackgroundStyle(wxBackgroundStyle::wxBG_STYLE_PAINT);
	m_gameView->SetBackgroundColour(wxColour(0, 0, 0));
	m_gameView->SetMinSize(wxSize(boardDim, boardDim));
	m_gameView->SetMaxSize(wxSize(boardDim, boardDim));

	bSizer1->Add(m_gameView, 0, wxALL | wxALIGN_CENTRE_HORIZONTAL, 5);

	//TODO: fix list 
	m_playerList = new wxListView(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_ICON | wxLC_NO_HEADER);
	auto images = new wxImageList(24, 24, true, 4);
	wxMemoryDC memDc;
	for (int i = 0; i < 4; i++)
	{
		auto bitmap = images->GetBitmap(i);
		memDc.SelectObject(bitmap);
		memDc.FloodFill(0,0, s_brushes[i + 1]->GetColour());
	}
	m_playerList->SetMaxSize(wxSize(boardDim, 32));
	m_playerList->SetMinSize(wxSize(boardDim, 32));
	
	m_playerList->AssignImageList(images, wxIMAGE_LIST_NORMAL);

	bSizer1->Add(m_playerList, 0, wxALL|wxEXPAND, 5);

	wxBoxSizer* bSizer2 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticBoxSizer* sbSizer2 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Scores")), wxHORIZONTAL);

	m_scorelist = new wxListBox(sbSizer2->GetStaticBox(),wxID_ANY);
	sbSizer2->Add(m_scorelist, 1, wxALL| wxEXPAND, 5);

	m_historyButton = new wxButton(sbSizer2->GetStaticBox(), BTN_HISTORY, wxT("History"), wxDefaultPosition, wxDefaultSize, 0);
	sbSizer2->Add(m_historyButton, 0, wxALL, 5);

	bSizer2->Add(sbSizer2, 1, wxALL | wxEXPAND, 5);

	wxStaticBoxSizer* sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Game")), wxVERTICAL);

	m_nickName = new wxTextCtrl(sbSizer1->GetStaticBox(), wxID_ANY);
	m_nickName->SetMaxLength(14);
	m_nickName->SetHint(wxT("nick name"));
	m_nickName->SetValue(GenerateNickName());

	sbSizer1->Add(m_nickName, 0, wxALL|wxEXPAND, 5);

	wxBoxSizer* bSizer5 = new wxBoxSizer(wxHORIZONTAL);

	m_connectButton = new wxButton(sbSizer1->GetStaticBox(), BTN_CONNECT, wxT("Connect"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer5->Add(m_connectButton, 1, wxALL, 5);

	m_hotsButton = new wxButton(sbSizer1->GetStaticBox(), BTN_HOST, wxT("Host"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer5->Add(m_hotsButton, 1, wxALL, 5);

	sbSizer1->Add(bSizer5, 1, wxEXPAND, 5);

	m_disconnectButton = new wxButton(sbSizer1->GetStaticBox(), BTN_DISCONNECT, wxT("Disconnected"), wxDefaultPosition, wxDefaultSize, 0);
	m_disconnectButton->Enable(false);
	sbSizer1->Add(m_disconnectButton, 0, wxALL | wxEXPAND, 5);

	bSizer2->Add(sbSizer1, 0, wxALL | wxEXPAND, 5);

	bSizer1->Add(bSizer2, 0, wxEXPAND, 0);

	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);

	this->Centre(wxBOTH);

	// Connect Events
	m_gameView->Connect(wxEVT_PAINT, wxPaintEventHandler(MainFrame::PaintGame), NULL, this);
	m_connectButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::ConnectToRemote), NULL, this);
	m_hotsButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::StartAsHost), NULL, this);
	m_disconnectButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::DisconnectGame), NULL, this);
	m_historyButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::ShowHistory), NULL, this);
	m_gameView->Connect(wxID_ANY, wxEVT_KEY_DOWN, wxKeyEventHandler(MainFrame::ProcessKeyInput), 0, this);
	Bind(ENGINE_EVENT, &MainFrame::OnEngineEvent, this);
	Bind(wxEVT_TIMER, &MainFrame::ProcessTick, this);
	Bind(wxEVT_SOCKET, &MainFrame::ProcessSocket, this);
}

MainFrame::~MainFrame()
{
	if (m_game) m_game->Destroy();
	// Disconnect Events
	m_gameView->Disconnect(wxEVT_PAINT, wxPaintEventHandler(MainFrame::PaintGame), NULL, this);
	m_connectButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::ConnectToRemote), NULL, this);
	m_hotsButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::StartAsHost), NULL, this);
	m_disconnectButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::DisconnectGame), NULL, this);
	m_historyButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::ShowHistory), NULL, this);
	m_gameView->Disconnect(wxID_ANY, wxEVT_KEY_DOWN, wxKeyEventHandler(MainFrame::ProcessKeyInput), 0, this);
	Unbind(ENGINE_EVENT, &MainFrame::OnEngineEvent, this);
	Unbind(wxEVT_TIMER, &MainFrame::ProcessTick, this);
	Unbind(wxEVT_SOCKET, &MainFrame::ProcessSocket, this);
}

void MainFrame::OnEngineEvent(EngineEvent& event)
{
	auto eevt = event.GetEngineEvent();
	if (eevt == EET_REFRESH)
	{
		// client event
		m_gameView->Refresh(false);
	}
	else if (eevt == EET_PLAYERS)
	{
		m_playerList->DeleteAllItems();
		PlayerMap& players = m_game->GetPlayers();
		int idx = 0;
		for (auto i = players.begin(), j = players.end(); i != j; i++)
		{
			m_playerList->InsertItem(idx, i->second->nick, i->second->color-1);
			m_playerList->EnsureVisible(idx++);
		}
	}
	else if (eevt == EET_ASK_READY)
	{
		AskReady();
	}
	else if (eevt == EET_START)
	{
		// host specific
		m_game->Start();
	}
	else if (eevt == EET_END)
	{
		m_scorelist->Clear();
		PlayerMap& players = m_game->GetPlayers();
		wxString score;
		for (auto i = players.begin(), j = players.end(); i != j; i++)
		{
			Player& p = *i->second;
			if (!score.empty()) score << wxT(" | ");
			wxString playerScore = wxString() << p.nick << wxT(": ") << p.snake.size();
			score << playerScore;
			m_scorelist->Append(playerScore);

		}
		// store score
		m_hist.Add(score);
		m_hist.Save();
		
		AskReady();
	}
	else if (eevt == EET_TERMINATE)
	{
		_Terminate();
	}
}

void MainFrame::_Terminate()
{
	// destroy game 
	m_game->Destroy();
	m_game = NULL;
	_EnableConnectButtons(true);
}

void MainFrame::AskReady()
{
	int ret = wxMessageBox(wxT("Hit Ok when ready?"), wxT("Ready Check"), wxOK|wxCANCEL|wxICON_QUESTION);
	if(ret==wxOK) m_game->Ready();
	else _Terminate();
}

void MainFrame::ProcessKeyInput(wxKeyEvent& event)
{
	if (m_game && m_game->IsPlaying())
	{
		int code = event.GetKeyCode();
		switch (code)
		{
		case WXK_UP:
			m_game->Turn(DIR_UP);
			break;
		case WXK_DOWN:
			m_game->Turn(DIR_DOWN);
			break;
		case WXK_LEFT:
			m_game->Turn(DIR_LEFT);
			break;
		case WXK_RIGHT:
			m_game->Turn(DIR_RIGHT);
			break;
		default:
			return;
		}
	}
}

void MainFrame::ProcessTick(wxTimerEvent& event)
{
	m_game->ProcessTick();
	m_gameView->Refresh(false);
}

void MainFrame::ProcessSocket(wxSocketEvent& event)
{
	if (m_game) m_game->ProcessSocket(event);
}

void MainFrame::PaintGame(wxPaintEvent& event)
{
	wxAutoBufferedPaintDC dc(m_gameView);
	if (m_game)
	{
		dc.SetBackground(*s_brushes[0]);
		dc.Clear();

		// /draw snakes
		for (auto i = m_game->GetPlayers().begin(), j = m_game->GetPlayers().end(); i != j; i++)
		{
			for (auto q = i->second->snake.begin(), t = i->second->snake.end(); q != t; q++)
			{
				int color = i->second->color;
				dc.SetPen(*s_pens[color]);
				dc.SetBrush(*s_brushes[color]);
				dc.DrawRectangle(q->x * GAME_CELL_SIZE, q->y * GAME_CELL_SIZE, GAME_CELL_SIZE, GAME_CELL_SIZE);
			}
		}
		// draw food
		for (auto i = m_game->GetFood().begin(), j = m_game->GetFood().end(); i != j; i++)
		{
			dc.SetPen(*s_pens[0]);
			dc.GradientFillConcentric(wxRect(i->second.x * GAME_CELL_SIZE, i->second.y * GAME_CELL_SIZE, GAME_CELL_SIZE, GAME_CELL_SIZE), s_foodColor, *wxBLACK);
		}
	}
	else
	{
		dc.Clear();
		int linePixels = GAME_LINE_SIZE * GAME_CELL_SIZE;
		dc.SetPen(*s_pens[1]);
		dc.SetBrush(*s_brushes[1]);
		for (int y = 0; y < linePixels; y += GAME_CELL_SIZE)
		{
			dc.DrawRectangle(y, y, GAME_CELL_SIZE, GAME_CELL_SIZE);
			dc.DrawRectangle(linePixels - y - GAME_CELL_SIZE, y, GAME_CELL_SIZE, GAME_CELL_SIZE);
		}
	}
}

void MainFrame::_CreateGame(EngineType mode)
{
	auto nick = m_nickName->GetValue().Trim();
	if (nick.IsEmpty())
	{
		wxMessageBox(wxT("No nick name."), wxT("Error"));
		return;
	}

	ConnectionDialog dlg(this, mode, m_cfg);
	if (dlg.ShowModal() == wxID_OK)
	{
		_EnableConnectButtons(false);
		m_game = EngineFactory::Create(mode, dlg.GetValue(), this, nick);
		m_gameView->Refresh();
	}
}

void MainFrame::ConnectToRemote(wxCommandEvent& event)
{
	_CreateGame(ET_CLIENT);
}

void MainFrame::StartAsHost(wxCommandEvent& event)
{
	_CreateGame(ET_HOST);
}

void MainFrame::DisconnectGame(wxCommandEvent& event)
{
	if (m_game)
	{
		m_game->Destroy();
		m_game = NULL;
	}
	_EnableConnectButtons(true);
	m_gameView->Refresh();
}

void MainFrame::ShowHistory(wxCommandEvent& event)
{
	HistoryDialog dlg(this, m_hist);
	dlg.ShowModal();
}

void MainFrame::_EnableConnectButtons(bool enabled)
{
	m_hotsButton->Enable(enabled);
	m_connectButton->Enable(enabled);
	m_disconnectButton->Enable(!enabled);
}