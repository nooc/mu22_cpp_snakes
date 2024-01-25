#include <wx/wxprec.h>
#include <wx/socket.h>
#include <cmath>
#include <random>

#include "engine.h"

// define custom event. (outside namespace to prevent link error)
wxDEFINE_EVENT(ENGINE_EVENT, snakes::EngineEvent);

using namespace snakes;

// socket event flags
#define SOCKET_FLAGS wxSOCKET_INPUT_FLAG | wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG

EngineEvent::EngineEvent(EngineEventType type, int winId) : m_type(type), wxEvent(winId, ENGINE_EVENT) { }
EngineEvent::EngineEvent(const EngineEvent& event) : wxEvent(event)
{
	m_type = event.m_type;
}
wxEvent* EngineEvent::Clone() const { return new EngineEvent(*this); }
EngineEventType EngineEvent::GetEngineEvent() { return m_type; }
// implement rtti for custom event
wxIMPLEMENT_DYNAMIC_CLASS(EngineEvent, wxEvent);

// internal player structure
struct PlayerImpl : Player
{
	wxSocketBase* m_socket;

	PlayerImpl(short _id, short x, short y, short _dir, wxSocketBase* socket): m_socket(socket),Player(_id, x, y, _dir) {}
	~PlayerImpl() { if (m_socket) m_socket->Destroy(); }
};

/// <summary>
/// Get sockaddr from connection string "HOST:PORT"
/// </summary>
/// <param name="connStr"></param>
/// <returns></returns>
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

// internal base implementation of Engine
struct SnakeEngine : Engine
{
	// send game ticks
	wxTimer m_timer;
	// event target
	wxEvtHandler* m_handler;
	// game board of cells
	std::vector<char> m_board;
	// map of players
	PlayerMap m_players;
	// convenience pointer to local player object (included in m_players)
	PlayerImpl* m_me;
	bool m_ok;
	int m_boardIdCounter;

	std::random_device m_rd;
	std::mt19937 m_gen; // mersenne_twister_engine seeded with rd()

	const int m_cellCount;

	SnakeEngine(wxEvtHandler* handler)
		: m_handler(handler), m_boardIdCounter(1),
		m_board(std::pow(GAME_CELL_SIZE * GAME_LINE_SIZE,2)),
		m_ok(false),
		m_gen(m_rd()),
		m_cellCount(GAME_LINE_SIZE* GAME_LINE_SIZE),
		m_timer(handler)
	{
		ResetBoard();
	}
	virtual ~SnakeEngine()
	{
		m_timer.Stop();
	}

	/// <summary>
	/// Set board cells to zero.
	/// </summary>
	void ResetBoard()
	{
		char* b = m_board.data();
		for (int i = 0, j = m_board.capacity(); i < j; i++) b[i] = 0;
	}

	void ArrangePlayers() {
		int playerCount = m_players.size();
		int quat = GAME_LINE_SIZE / 4;
		int half = GAME_LINE_SIZE / 2;
		int ystart = playerCount < 3 ? half : quat;
		int iterPos = 0;
		for (auto i = m_players.begin(), j = m_players.end(); i != j; i++)
		{
			int row = iterPos / 2;
			int col = iterPos % 2;
			auto pos = i->second->snake.front();
			pos.x = quat + col*half;
			pos.y = ystart + col*half;
		}
	}

	/// <summary>
	/// Add a player to the game and readjust starting positions if >= 2.
	/// If socket is null, this is the local player
	/// </summary>
	void AddPlayer(wxSocketBase*socket)
	{
		int mapId = socket ? socket->GetSocket() : 0;
		std::uniform_int_distribution<> distrib(0, 3);
		auto player = new PlayerImpl(m_boardIdCounter++, GAME_LINE_SIZE / 2, GAME_LINE_SIZE / 2, distrib(m_gen), socket);
		PlayerPtr ptr(player);
		m_players[mapId] = ptr;
		if (!socket) m_me = player;
		// adjust positions if 2, 3 or 4 players
		if (m_players.size() > 1) ArrangePlayers();
	}

	PlayerMap& GetPlayers() {
		return m_players;
	}

	void AddFood()
	{
		std::uniform_int_distribution<> distrib(0, m_cellCount-1);
		char* cellArray = m_board.data();
		int cell;
		do
		{
			cell = distrib(m_gen);
		} while (cellArray[cell] != 0);
		cellArray[cell] = FOOD_VALUE;
	}

	char* GetBoard()
	{
		return m_board.data();
	}

	bool IsOk() { return m_ok; }

	void ReadData(wxSocketBase* sock)
	{
		Message m;
		PlayerImpl& p = (PlayerImpl&)*m_players[sock->GetSocket()];
		sock->Read(&m, sizeof(MessageHeader));
		switch (m.header.type)
		{
			gfsgf
		default:
			break;
		}
	}
};

/// <summary>
/// Internal host implementation extending SnakeEngine.
/// </summary>
struct HostSnakeEngine : SnakeEngine
{
	wxSocketServer* m_listening;

	HostSnakeEngine(const wxString& connectionString, wxEvtHandler* handler) : SnakeEngine(handler)
	{
		m_listening = new wxSocketServer(getAddr(connectionString));
		m_listening->SetNotify(SOCKET_FLAGS);
		m_listening->SetEventHandler(*handler);
		m_listening->Notify(true);
		m_ok = m_listening->IsOk();
		AddPlayer(NULL);

		wxPostEvent(m_handler, EngineEvent(EngineEventType::EET_ASK_READY));
	}
	
	~HostSnakeEngine()
	{
		delete m_listening;
		m_players.clear();
	}

	void Destroy()
	{
		delete this;
	}

	void RemovePlayer(wxSocketBase* sock)
	{
		m_players.erase(sock->GetSocket());
	}

	void ProcessSocket(wxSocketEvent& event)
	{
		auto sock = event.GetSocket();
		switch (event.GetSocketEvent())
		{
		case wxSOCKET_INPUT:
			ReadData(sock);
			break;
		case wxSOCKET_LOST:
			RemovePlayer(sock);
			break;
		case wxSOCKET_CONNECTION:

			if (m_players.size() < 4)
			{
				AddPlayer(sock);
			}
			else sock->Destroy();
			break;
		default:
			break;
		}
	}

	int GetCellValue(const BoardPos& pos)
	{
		return m_board.at(pos.y * GAME_LINE_SIZE + pos.x);
	}
	void SetCellValue(const BoardPos& pos, int value)
	{
		m_board[pos.y * GAME_LINE_SIZE + pos.x] = (char)value;
	}

	BoardPos NextPos(const BoardPos& pos, int dir, char* board)
	{
		int addX = 0, addY = 0;
		if (dir == 0) { addX = 1; }
		else if (dir == 1) { addY = -1; }
		else if (dir == 2) { addX = -1; }
		else if (dir == 3) { addY = 1; }
		return BoardPos((pos.x + addX) % GAME_LINE_SIZE, (pos.y + addY) % GAME_LINE_SIZE);
	}

	void SendDead(PlayerImpl& player)
	{
		if (player.m_socket)
		{
			Message m;
			m.header.type = MessageType::DEAD;
			m.header.size = 0;
			player.m_socket->Write(&m, sizeof(MessageHeader));
		}
	}

	/// <summary>
	/// Send players advance message
	/// </summary>
	/// <param name="player"></param>
	void SendAdvance(PlayerImpl& player)
	{
		Message m;
		auto pos = player.snake.back();
		m.header.type = MessageType::ADVANCE;
		m.header.size = sizeof(AdvanceMessage);
		m.body.advance.grow = player.grow;
		m.body.advance.pos.id = player.id;
		m.body.advance.pos.x = pos.x;
		m.body.advance.pos.y = pos.y;

		auto i = m_players.begin(), j = m_players.end();
		while (i != j)
		{
			PlayerImpl& p = (PlayerImpl&)*i->second;
			if (!p.m_socket) continue;
			p.m_socket->Write(&m, sizeof(m.header) + m.header.size);
		}
	}

	/// <summary>
	/// Send round end
	/// </summary>
	void SendEnd()
	{
		Message m;
		m.header.type = MessageType::END;
		m.header.size = sizeof(EndMessage) -(sizeof(PlayerScore)*(4-m_players.size())); // only send scores eq to amount of players (max 4)
		m.body.end.count = m_players.size();
		auto i = m_players.begin(), j = m_players.end();
		int pIdx = 0;
		while (i != j)
		{
			PlayerScore& score = m.body.end.score[pIdx++];
			score.id = i->second->id;
			score.score = i->second->snake.size();
			i++;
		}
		m.body.end.count = pIdx; // player count
		i = m_players.begin();
		while (i != j)
		{
			PlayerImpl* p = (PlayerImpl*)i->second.get();
			if (p->m_socket)
			{
				p->m_socket->Write(&m, sizeof(MessageHeader) + m.header.size);
			}
		}
	}

	void ProcessTick()
	{
		char* board = m_board.data();
		auto i = m_players.begin(), j = m_players.end();
		while (i != j)
		{
			// get player object
			PlayerImpl& player = (PlayerImpl&)(*i->second);
			if (!player.alive) continue;
			// next cell
			BoardPos next = NextPos(player.snake.back(), player.dir, board);
			int cell = GetCellValue(next);
			bool food = cell == FOOD_VALUE;
			// dead
			if (!food && cell != 0)
			{
				player.alive = false;
				SendDead(player);
				continue;
			}
			// advance
			player.snake.push_back(next);
			// send to clients
			SendAdvance(player);
			// pop oldest cell if not growing
			if (!player.grow) player.snake.pop_front();
			player.grow = food;
			// next cell
			i++;
		}
		// continue if anyone is alive
		i = m_players.begin();
		while (i != j) {
			if (i->second->alive) return;
			i++;
		}
		// nobody alive. end
		SendEnd();
	}
	
	/// <summary>
	/// Mark self ready and test everyone.
	/// </summary>
	void Ready() {
		m_me->ready = true;
		TestReady();
	}

	/// <summary>
	/// Test if everyone is ready and start if so.
	/// </summary>
	void TestReady()
	{
		auto i = m_players.begin(), j = m_players.end();
		if (i != j)
		{
			if (!i->second->ready) return;
			i++;
		}
		m_timer.Start(333);
	}

	bool IsPlaying() {
		return false;
	}

	/// <summary>
	/// Turn if the turn is 90 degrees.
	/// </summary>
	/// <param name="dir"></param>
	void Turn(Direction dir)
	{
		if ((m_me->dir + 2) % 4 != dir) m_me->dir = dir;
	}
};

/// <summary>
/// Internal client implementation extending SnakeEngine
/// </summary>
struct ClientSnakeEngine : SnakeEngine
{
	wxSocketClient* m_connection;

	ClientSnakeEngine(const wxString& connectionString, wxEvtHandler* handler) : SnakeEngine(handler)
	{
		m_connection = new wxSocketClient();
		m_connection->SetNotify(SOCKET_FLAGS);
		m_connection->SetEventHandler(*handler);
		m_connection->Notify(true);
		m_connection->SetTimeout(3);
		m_connection->Connect(getAddr(connectionString));
		m_ok = m_connection->IsOk();
		AddPlayer(NULL);
	}
	~ClientSnakeEngine()
	{
		delete m_connection;
	}

	void Destroy()
	{
		delete this;
	}

	void ProcessSocket(wxSocketEvent& event)
	{
		auto sock = event.GetSocket();
		switch (event.GetSocketEvent())
		{
		case wxSOCKET_INPUT:
			ReadData(sock);
			break;
		case wxSOCKET_LOST:
			wxPostEvent(m_handler, EngineEvent(EngineEventType::EET_TERMINATE));
			break;
		default:
			break;
		}
	}

	bool IsPlaying() {
		return false;
	}

	void ProcessTick()
	{
		// client only reacts to remote
	}

	void Ready() {
		Message m;
		m.header.type = MessageType::READY;
		m.header.size = 0;
		m_connection->Write(&m, sizeof(m.header));
	}

	/// <summary>
	/// Turn if the turn is 90 degrees.
	/// </summary>
	/// <param name="dir"></param>
	void Turn(Direction dir)
	{
		if ((m_me->dir + 2) % 4 != dir)
		{
			Message m;
			m.header.type = MessageType::TURN;
			m.header.size = sizeof(TurnMessage);
			m.body.turn.dir = dir;
			m_connection->Write(&m, sizeof(m.header) + m.header.size);
		}
	}
};

/// <summary>
/// Factory for creating engine instance
/// </summary>
/// <param name="type"></param>
/// <param name="connectionString"></param>
/// <param name="handler"></param>
/// <returns></returns>
Engine* EngineFactory::Create(EngineType type, const wxString& connectionString, wxEvtHandler* handler)
{
	return (type == ET_CLIENT) ? (Engine*)new ClientSnakeEngine(connectionString, handler) : (Engine*)new HostSnakeEngine(connectionString, handler);
}
