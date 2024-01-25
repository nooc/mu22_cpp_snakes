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
		END, // server->clientS end round
		CHAT // client->server chat, server->clientS
	};

	// Move direction
	enum Direction
	{
		DIR_RIGHT,
		DIR_UP,
		DIR_LEFT,
		DIR_DOWN
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
		wxUint16 x, y;
	};

	struct IDMessage
	{
		int id;
	};

	struct PlayersMessage
	{
		wxUint16 count;
		PosMessage players[4];
	};

	struct TurnMessage
	{
		Direction dir; // TurnDirection
	};

	struct AdvanceMessage
	{
		PosMessage pos;
		bool grow;
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
		} body;
	};
}