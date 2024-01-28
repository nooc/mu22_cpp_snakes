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

#define TIMER_INTERVALL 120

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

	PlayerImpl(short _id, short x, short y, Direction _dir, wxSocketBase* socket): m_socket(socket),Player(_id, x, y, _dir) {}
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
	// event target
	wxEvtHandler* m_handler;
	// game board of cells
	char* m_board;
	int m_boardSize;
	// map of players
	PlayerMap m_players;
	// food on board
	FoodMap m_food;
	// convenience pointer to local player object (included in m_players)
	PlayerImpl* m_me;
	bool m_ok;
	bool m_host;
	bool m_playing;

	std::random_device m_rd;
	std::mt19937 m_gen; // mersenne_twister_engine seeded with rd()

	SnakeEngine(wxEvtHandler* handler, bool host=false)
		: m_handler(handler),
		m_host(host),
		m_ok(false),
		m_playing(false),
		m_gen(m_rd())
	{
		m_boardSize = GAME_LINE_SIZE * GAME_LINE_SIZE;
		m_board = new char[m_boardSize];
		ResetBoard();
	}
	virtual ~SnakeEngine()
	{
		delete[] m_board;
	}

	virtual void ProcessTick() {}

	void ConfigSocket(wxSocketBase* sock)
	{
		sock->SetNotify(SOCKET_FLAGS);
		sock->SetEventHandler(*m_handler);
		sock->Notify(true);
		sock->SetTimeout(3);
	}

	/// <summary>
	/// Set board cells to zero.
	/// </summary>
	void ResetBoard()
	{
		for (int i = 0, j = m_boardSize; i < j; i++) m_board[i] = 0;
		m_food.clear();
	}

	FoodMap& GetFood() const
	{
		return const_cast<FoodMap&>(m_food);
	}

	/// <summary>
	/// move players to appropriate starting positions and set color
	/// </summary>
	void ArrangePlayers() {
		int quat = GAME_LINE_SIZE / 4;
		int half = GAME_LINE_SIZE / 2;
		int iterPos = 0;
		for (auto i = m_players.begin(), j = m_players.end(); i != j; i++)
		{
			int row = iterPos / 2;
			int col = iterPos % 2;
			i->second->snake.back().set(quat + col* half, quat + row * half);
			i->second->color = 1 + iterPos;
			iterPos++;
		}
		SendPlayers();
	}

	/// <summary>
	/// Add a player to the game and then call ArrangePlayers().
	/// </summary>
	/// <param name="id">Player id. Same as socket id on host side.</param>
	/// <param name="socket">Socket or null</param>
	/// <returns>Created player</returns>
	PlayerImpl* AddPlayer(int id, wxSocketBase*socket)
	{
		std::uniform_int_distribution<> distrib(0, 3);
		auto player = new PlayerImpl(id, GAME_LINE_SIZE / 2, GAME_LINE_SIZE / 2, (Direction)distrib(m_gen), socket);
		PlayerPtr ptr(player);
		m_players[id] = ptr;
		// send id
		if (m_host && socket)
		{
			Message m;
			m.header.type = MessageType::ID;
			m.header.size = sizeof(IDMessage);
			m.body.id.id = ptr->id;
			socket->Write(&m, sizeof(MessageHeader) + m.header.size);
		}
		// adjust positions if 2, 3 or 4 players
		if (m_host && m_players.size() > 1) ArrangePlayers();
		return player;
	}

	/// <summary>
	/// Get player list from host.
	/// </summary>
	/// <param name="m"></param>
	void GetPlayerList(PlayersMessage& m)
	{
		ResetBoard();
		for (int i = 0; i < m.count; i++)
		{
			PlayerMessage& pm = m.player[i];
			PlayerImpl* p;
			auto pptri = m_players.find(pm.id);
			if (pptri == m_players.end())
				p = AddPlayer(pm.id, NULL);
			else p = (PlayerImpl*)pptri->second.get();
			p->snake.clear();
  			p->snake.push_back(pm.pos);
			p->color = pm.color;
			p->dir = pm.dir;
			m_board[pm.pos.cell(GAME_LINE_SIZE)] = pm.color;
		}
	}

	/// <summary>
	/// Send player list to clients.
	/// </summary>
	void SendPlayers()
	{
		Message m;
		m.body.players.count = m_players.size();
		m.header.type = MessageType::PLAYERS;
		m.header.size = sizeof(m.body.players.count) + sizeof(PlayerMessage) * m.body.players.count; // only send pos eq to amount of players (max 4)
		m.body.players.count = m_players.size();
		int idx = 0;
		for (auto i = m_players.begin(), j = m_players.end(); i != j; i++)
		{
			PlayerMessage& pcm = m.body.players.player[idx++];
			pcm.id = i->second->id;
			pcm.pos = i->second->snake.back();
			pcm.color = i->second->color;
		}
		for (auto i = m_players.begin(), j = m_players.end(); i != j; i++)
		{
			PlayerImpl& p = (PlayerImpl&)(*i->second);
			if (p.m_socket) p.m_socket->Write(&m, sizeof(MessageHeader) + m.header.size);
		}
	}

	PlayerMap& GetPlayers() {
		return m_players;
	}

	void AddFood()
	{
		std::uniform_int_distribution<> distrib(0, m_boardSize-1);
		int cell;
		do
		{
			cell = distrib(m_gen);
		} while (m_board[cell] != 0);
		Position foodPos;
		foodPos.set(cell % GAME_LINE_SIZE, cell / GAME_LINE_SIZE);
		m_board[cell] = FOOD_VALUE;
		m_food[cell] = foodPos;
		// send to clients
		Message m;
		m.header.type = MessageType::FOOD;
		m.header.size = sizeof(m.body.food);
		m.body.food.pos = foodPos;
		for (auto i = m_players.begin(), j = m_players.end(); i != j; i++)
		{
			PlayerImpl& p = (PlayerImpl&)*i->second;
			if (p.m_socket) p.m_socket->Write(&m,sizeof(MessageHeader)+m.header.size);
		}
	}

	char* GetBoard() const
	{
		return m_board;
	}

	bool IsOk() { return m_ok; }

	virtual void TestReady() {}

	void EchoTurn(PlayerImpl& player, Message &m)
	{
		m.body.turn.request = false;
		player.m_socket->Write(&m, sizeof(MessageHeader)+m.header.size);
	}

	/// <summary>
	/// Advance snakes.
	/// </summary>
	/// <param name="advance"></param>
	void DoAdvance(AdvanceMessage& advance)
	{
		for (int i = 0; i < advance.count; i++)
		{
			auto playerPtr = m_players.find(advance.posgrow[i].id);
			if (playerPtr == m_players.end()) continue;

			PosGrow& pg = advance.posgrow[i];
			PlayerImpl& player = (PlayerImpl&)*playerPtr->second;
			player.snake.push_back(pg.pos);
			int cell = pg.pos.cell(GAME_LINE_SIZE);
			m_board[cell] = player.color;
			if (!pg.grow)
			{
				Position& old = player.snake.front();
				m_board[old.cell(GAME_LINE_SIZE)] = 0;
				player.snake.pop_front();
			}
			// remove food from list
			if (m_food.find(cell) != m_food.end()) m_food.erase(cell);
		}
		wxPostEvent(m_handler, EngineEvent(EngineEventType::EET_REFRESH));
	}

	void PlaceFood(FoodMessage& food)
	{
		int cell = food.pos.cell(GAME_LINE_SIZE);
		m_board[cell] = FOOD_VALUE;
		m_food[cell] = food.pos;
	}

	void ReadData(wxSocketBase* sock)
	{
		Message m;
		PlayerImpl* p;
		auto ppti = m_players.find(sock->GetSocket());
		p = ppti == m_players.end() ? NULL : (PlayerImpl*)ppti->second.get();

		sock->Read(&m, sizeof(MessageHeader));
		if(m.header.size!=0) sock->Read(&m.body, m.header.size);
		switch (m.header.type)
		{
		case MessageType::ADVANCE:
			DoAdvance(m.body.advance);
			break;
		case MessageType::FOOD:
			PlaceFood(m.body.food);
			break;
		case MessageType::DEAD:
			if(p) p->alive = false;
			break;
		case MessageType::START:
			m_playing = true;
			break;
		case MessageType::END:
			m_playing = false;
			for (auto i = m_players.begin(), j = m_players.end(); i != j; i++)
			{
				i->second->grow = false;
				i->second->ready = false;
			}
			wxPostEvent(m_handler, EngineEvent(EngineEventType::EET_END));
			break;
		case MessageType::ID:
			m_me = AddPlayer(m.body.id.id, NULL); // cient adds self
			wxPostEvent(m_handler, EngineEvent(EngineEventType::EET_ASK_READY));
			break;
		case MessageType::PLAYERS:
			GetPlayerList(m.body.players);
			break;
		case MessageType::POS:
			m_me->snake.back() = m.body.position.pos;
			break;
		case MessageType::READY:
			m_players[sock->GetSocket()]->ready = true;
			TestReady();
			break;
		case MessageType::TURN:
			if (m.body.turn.request && p)
			{
				p->dir = m.body.turn.dir;
				EchoTurn(*p, m);
			}
			else
			{
				m_me->dir = m.body.turn.dir;
			}
			break;
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
	// send game ticks
	wxTimer m_timer;

	HostSnakeEngine(const wxString& connectionString, wxEvtHandler* handler) : SnakeEngine(handler,true), m_timer(handler)
	{
		m_listening = new wxSocketServer(getAddr(connectionString));
		m_listening->SetNotify(SOCKET_FLAGS);
		m_listening->SetEventHandler(*handler);
		m_listening->Notify(true);
		m_ok = m_listening->IsOk();
		m_me = AddPlayer(m_listening->GetSocket(), NULL);

		wxPostEvent(m_handler, EngineEvent(EngineEventType::EET_ASK_READY));
	}
	
	~HostSnakeEngine()
	{
		m_timer.Stop();
		delete m_listening;
		m_players.clear();
	}
	void Start()
	{
		m_timer.Start(TIMER_INTERVALL);
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
		switch (event.GetSocketEvent())
		{
		case wxSOCKET_INPUT:
			ReadData(event.GetSocket());
			break;
		case wxSOCKET_LOST:
			RemovePlayer(event.GetSocket());
			break;
		case wxSOCKET_CONNECTION:

			if (m_players.size() < 4)
			{
				wxSocketBase* sock = m_listening->Accept(false);
				if (sock)
				{
					ConfigSocket(sock);
					AddPlayer(sock->GetSocket(), sock);
				}
			}
			break;
		default:
			break;
		}
	}

	Position NextPos(const Position& pos, int dir)
	{
		Position p;
		int addX = 0, addY = 0;
		if (dir == 0) { addX = 1; }
		else if (dir == 1) { addY = -1; }
		else if (dir == 2) { addX = -1; }
		else if (dir == 3) { addY = 1; }
		p.x = (pos.x + addX + GAME_LINE_SIZE) % GAME_LINE_SIZE;
		p.y = (pos.y + addY + GAME_LINE_SIZE) % GAME_LINE_SIZE;
		return p;
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
	/// Send players advance message to all clients
	/// </summary>
	/// <param name="player"></param>
	void SendAdvance()
	{
		Message m;
		m.header.type = MessageType::ADVANCE;
		m.header.size = sizeof(m.body.advance.count) + m_players.size()*sizeof(PosGrow);

		int idx = 0;
		for (auto i = m_players.begin(), j = m_players.end(); i != j; i++)
		{
			PlayerImpl& p = (PlayerImpl&)*i->second;
			if (!p.alive) continue;
			m.body.advance.posgrow[idx].grow = p.grow;
			m.body.advance.posgrow[idx].id = p.id;
			m.body.advance.posgrow[idx].pos = p.snake.back();
			idx++;
		}
		m.body.advance.count = idx;
		for (auto i = m_players.begin(), j = m_players.end(); i != j; i++)
		{
			PlayerImpl& p = (PlayerImpl&)*i->second;
			if (p.m_socket) p.m_socket->Write(&m, sizeof(m.header) + m.header.size);
		}
	}

	/// <summary>
	/// Send round end
	/// </summary>
	void SendEnd()
	{
		Message m;
		m.header.type = MessageType::END;
		m.header.size = sizeof(m.body.end.count)  + sizeof(PlayerScore)*m_players.size(); // only send scores eq to amount of players (max 4)
		m.body.end.count = m_players.size();
		int pIdx = 0;
		for (auto i = m_players.begin(), j = m_players.end(); i != j; i++)
		{
			// reset
			i->second->grow = false;
			i->second->ready = false;
			// set score
			PlayerScore& score = m.body.end.score[pIdx++];
			score.id = i->second->id;
			score.score = i->second->snake.size();
		}
		m.body.end.count = pIdx; // player count
		for (auto i = m_players.begin(), j = m_players.end(); i != j; i++)
		{
			PlayerImpl& p = (PlayerImpl&)*i->second;
			if (p.m_socket)
			{
				p.m_socket->Write(&m, sizeof(MessageHeader) + m.header.size);
			}
		}
	}

	void ProcessTick()
	{
		for (auto i = m_players.begin(), j = m_players.end(); i != j; i++)
		{
			// get player object
			PlayerImpl& player = (PlayerImpl&)(*i->second);
			if (!player.alive) continue;
			// next cell
			Position next = NextPos(player.snake.back(), player.dir);
			int cell = next.cell(GAME_LINE_SIZE);
			bool food = m_board[cell] == FOOD_VALUE;
			
			if (!food && m_board[cell] != 0)
			{
				// dead
				player.alive = false;
				SendDead(player);
				continue;
			}
			// advance
			player.snake.push_back(next);
			m_board[cell] = player.color;
			if (food)
			{
				m_food.erase(cell);
				AddFood();
			}
			// pop oldest cell if not growing
			if (!player.grow)
			{
				Position& old = player.snake.front();
				m_board[old.cell(GAME_LINE_SIZE)] = 0;
				player.snake.pop_front();
			}
			player.grow = food;
		}
		// send to clients
		SendAdvance();
		// continue if anyone is alive
		for (auto i = m_players.begin(), j = m_players.end(); i != j; i++)
		{
			if (i->second->alive) return;
		}
		// nobody alive. end
		m_playing = false;
		m_timer.Stop();
		SendEnd();
		wxPostEvent(m_handler, EngineEvent(EngineEventType::EET_END));
	}
	
	/// <summary>
	/// Mark self ready and test everyone.
	/// </summary>
	void Ready() {
		m_me->ready = true;
		TestReady();
	}

	/// <summary>
	/// Reset snakes
	/// </summary>
	void ResetSnakes()
	{
		for (auto i = m_players.begin(), j = m_players.end(); i != j; i++)
		{
			i->second->snake.clear();
			i->second->snake.push_back(Position());
			i->second->alive = true;
		}
	}

	/// <summary>
	/// Test if everyone is ready and start if so.
	/// </summary>
	void TestReady()
	{
		for (auto i = m_players.begin(), j = m_players.end(); i != j; i++)
		{
			// return if not ready
			if (!i->second->ready) return;
		}
		ResetSnakes();
		ResetBoard();
		ArrangePlayers();
		for (int i = 0; i < m_players.size(); i++) AddFood();
		SendStart();
		wxPostEvent(m_handler, EngineEvent(EngineEventType::EET_START));
	}

	bool IsPlaying() {
		return m_timer.IsRunning();
	}

	void SendStart()
	{
		Message m;
		m.header.type = MessageType::START;
		m.header.size = 0;
		for (auto i = m_players.begin(), j = m_players.end(); i != j; i++)
		{
			PlayerImpl& p = (PlayerImpl&)*i->second;

			if (p.m_socket) p.m_socket->Write(&m,sizeof(m.header));
		}
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
		ConfigSocket(m_connection);
		m_connection->Connect(getAddr(connectionString),false);
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
			m_ok = false;
			m_playing = false;
			wxPostEvent(m_handler, EngineEvent(EngineEventType::EET_TERMINATE));
			break;
		case wxSOCKET_CONNECTION:
			m_ok = true;
		default:
			break;
		}
	}

	bool IsPlaying() {
		return m_playing;
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
			m.body.turn.request = true;
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
