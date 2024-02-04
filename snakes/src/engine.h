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
		EET_PLAYERS,
		EET_ASK_READY,
		EET_START,
		EET_END,
		EET_TERMINATE,
		EET_REFRESH
	};

	class EngineEvent : public wxEvent
	{
		EngineEventType m_type;
	public:
		EngineEvent(): wxEvent(), m_type(EngineEventType::EET_REFRESH) {}
		EngineEvent(EngineEventType type, int winId=0);
		EngineEvent(const EngineEvent& event);
		~EngineEvent() {}
		wxEvent* Clone() const;
		EngineEventType GetEngineEvent();
		wxDECLARE_DYNAMIC_CLASS(EngineEvent);
	};

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

	struct Player
	{
		short id;
		short color;
		Direction dir;
		bool alive;
		bool ready;
		bool grow;
		wxString nick;
		std::list<Position> snake;

		Player(const wxString& _nick, short _id, short x, short y, Direction _dir)
			: nick(_nick),id(_id), color(0), dir(_dir), alive(false), grow(false), ready(false) {
			snake.push_back(Position().set(x, y));
		}
		virtual ~Player() {}

	};
	typedef std::shared_ptr<Player> PlayerPtr;
	typedef std::map<int, PlayerPtr> PlayerMap;
	typedef std::map<int, Position> FoodMap;

	struct Engine {
		virtual void ProcessSocket(wxSocketEvent& event) = 0;
		virtual void ProcessTick() = 0;
		virtual void Destroy() = 0;
		virtual bool IsOk() = 0;
		virtual bool IsPlaying() = 0;
		virtual PlayerMap& GetPlayers() const = 0;
		virtual FoodMap& GetFood() const = 0;
		virtual char* GetBoard() const = 0;
		virtual void Ready() = 0; // call from EET_ASK_READY
		virtual void Start() {} // call from EET_START event
		virtual void Turn(Direction dir) = 0;
	};

	struct EngineFactory
	{
		static Engine* Create(EngineType type, const wxString& connectionString, wxEvtHandler* handler, const wxString& nick);
	};
}
