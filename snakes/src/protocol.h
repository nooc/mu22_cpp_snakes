#pragma once

#include <wx/defs.h>

namespace snakes {

	enum MessageType
	{
		ID,	// server->client assign id
		POS, // server->client set start pos
		ADVANCE, // server->client advance snake
		DEAD, // server->client
		PLAYERS, // server->clientS announce players
		READY, // client->server mark as ready for game round
		TURN, // client->server request, server->clientS update
		START, // server->clientS end round
		END, // server->clientS end round
		FOOD // server->clientS food cell
	};

	// Move direction
	enum Direction
	{
		DIR_RIGHT,
		DIR_UP,
		DIR_LEFT,
		DIR_DOWN
	};

	struct Position
	{
		short x, y;
		Position& operator=(const Position& pos) { x = pos.x; y = pos.y; return *this; }
		Position& set(short _x, short _y) { x = _x; y = _y; return *this; }
	};
	
	// Player score where score is snake length.
	struct PlayerScore
	{
		int id;
		int score;
	};

	struct MessageHeader
	{
		MessageType type; // MessageType
		wxUint16 size; // byte size
	};

	struct PosMessage
	{
		int id;
		Position pos;
	};

	struct FoodMessage
	{
		Position pos;
	};

	struct PlayerMessage : PosMessage
	{
		int color;
		Direction dir;
	};

	struct IDMessage
	{
		int id;
	};

	struct PlayersMessage
	{
		wxUint16 count;
		PlayerMessage players[4];
	};

	struct TurnMessage
	{
		bool request;
		Direction dir; // TurnDirection
	};

	struct PosGrow : PosMessage
	{
		bool grow;
	};

	struct AdvanceMessage
	{
		wxUint16 count;
		PosGrow posgrow[4];
	};

	struct EndMessage
	{
		wxUint16 count;
		PlayerScore score[4];
	};

	struct Message
	{
		MessageHeader header;
		union {
			PlayersMessage players;
			IDMessage id;
			TurnMessage turn;
			EndMessage end;
			AdvanceMessage advance;
			PosMessage position;
			FoodMessage food;
		} body;
	};
}