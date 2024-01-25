#pragma once

#include <wx/string.h>

#include <wx/event.h>
#include <wx/socket.h>

#include <list>
#include <map>

#include "protocol.h"

namespace snakes { class EngineEvent; }

wxDECLARE_EVENT(ENGINE_EVENT, snakes::EngineEvent);

namespace snakes {
	enum EngineEventType
	{
		EET_UNDEFINED,
		EET_ASK_READY,
		EET_START,
		EET_END,
		EET_TERMINATE
	};

	class EngineEvent : public wxEvent
	{
		EngineEventType m_type;
	public:
		EngineEvent(): wxEvent(), m_type(EngineEventType::EET_UNDEFINED) {}
		EngineEvent(EngineEventType type, int winId=0);
		EngineEvent(const EngineEvent& event);
		~EngineEvent() {}
		wxEvent* Clone() const;
		EngineEventType GetEngineEvent();
		wxDECLARE_DYNAMIC_CLASS(EngineEvent);
	};
	//typedef void (wxEvtHandler::* EngineEventFunction)(EngineEvent&);

	// Size of a game block
	#define GAME_CELL_SIZE 10
	// Number of blocks making up a full horizontal or vertical line.
	#define GAME_LINE_SIZE 60
	// food
	#define FOOD_VALUE 10

	enum EngineType
	{
		ET_CLIENT,
		ET_HOST
	};

	struct BoardPos
	{
		short x, y;

		BoardPos(short _x, short _y) : x(_x),y(_y) {}
		BoardPos(const BoardPos& pos) : x(pos.x), y(pos.y) {}
	};

	struct Player
	{
		short id;
		Direction dir;
		bool alive;
		bool ready;
		bool grow;
		std::list<BoardPos> snake;

		Player(short _id, short x, short y, short _dir) : id(_id), dir(_dir), alive(false),grow(false),ready(false) {
			snake.push_back(BoardPos(x, y));
		}
		virtual ~Player() {}

	};
	typedef std::shared_ptr<Player> PlayerPtr;
	typedef std::map<int, PlayerPtr> PlayerMap;

	struct Engine {
		virtual void ProcessSocket(wxSocketEvent& event) = 0;
		virtual void ProcessTick() = 0;
		virtual void Destroy() = 0;
		virtual bool IsOk() = 0;
		virtual bool IsPlaying() = 0;
		virtual PlayerMap& GetPlayers() = 0;
		virtual char* GetBoard() = 0;
		virtual void Ready() = 0;
		virtual void Turn(Direction dir) = 0;
	};

	struct EngineFactory
	{
		static Engine* Create(EngineType type, const wxString& connectionString, wxEvtHandler* handler);
	};
}
