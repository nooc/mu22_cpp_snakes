#include <wx/wxprec.h>
#include <wx/socket.h>
#include <cmath>

#include "engine.h"

using namespace snakes;

static wxIPV4address getAddr(const wxString& connStr)
{
	wxIPV4address addr;
	wxString host;
	int port = 0;
	host = connStr.BeforeFirst(':');
	connStr.AfterFirst(':').ToInt(&port);
	if (!host.empty()) { addr.Hostname(host); }
	if (port > 0) { addr.Service(port); }
	return addr;
}

struct SnakeEngine : Engine
{
	wxEvtHandler* m_handler;
	wxWindow* m_gameComponent;
	std::vector<char> board;

	SnakeEngine(wxEvtHandler* handler) : m_handler(handler), board(std::pow(GAME_CELL_SIZE * GAME_LINE_SIZE,2))
	{
		
	}

	void SetGameComponent(wxWindow* component) {
		m_gameComponent = component;
	}
};

struct HostSnakeEngine : SnakeEngine
{
	wxSocketServer* m_listening;
	std::vector<wxSocketBase*> m_connected;

	HostSnakeEngine(const wxString& connectionString, wxEvtHandler* handler) : SnakeEngine(handler)
	{
		m_listening = new wxSocketServer(getAddr(connectionString), wxSOCKET_NOWAIT);
		m_listening->SetNotify(wxSOCKET_INPUT | wxSOCKET_OUTPUT | wxSOCKET_CONNECTION | wxSOCKET_LOST);
		m_listening->SetEventHandler(*handler);
	}
	
	~HostSnakeEngine()
	{
		delete m_listening;
		while (!m_connected.empty())
		{
			delete m_connected.back();
			m_connected.pop_back();
		}
	}

	void ProcessEvent(wxSocketEvent& event)
	{

	}

	void Destroy()
	{
		if (m_listening && m_listening->IsConnected())
		{
			m_listening->Close();
			auto socIter = m_connected.begin();
			while (socIter != m_connected.end())
			{
				(*socIter)->Close();
			}
		}
		delete this;
	}
};

struct ClientSnakeEngine : SnakeEngine
{
	wxSocketClient* m_connection;

	ClientSnakeEngine(const wxString& connectionString, wxEvtHandler* handler) : SnakeEngine(handler)
	{
		m_connection = new wxSocketClient(wxSOCKET_NOWAIT);
		m_connection->SetNotify(wxSOCKET_INPUT | wxSOCKET_OUTPUT | wxSOCKET_LOST);
		m_connection->SetEventHandler(*handler);
		m_connection->Connect(getAddr(connectionString), false);
	}
	~ClientSnakeEngine()
	{
		delete m_connection;
	}

	void ProcessEvent(wxSocketEvent& event)
	{

	}

	void Destroy()
	{
		m_connection->SetNotify(0);
		m_connection->Close();
	}
};

Engine* EngineFactory::Create(EngineType type, const wxString& connectionString, wxEvtHandler* handler)
{
	return (type == ET_CLIENT) ? (Engine*)new ClientSnakeEngine(connectionString, handler) : (Engine*)new HostSnakeEngine(connectionString, handler);
}
