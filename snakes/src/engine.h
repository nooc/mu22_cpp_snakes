#pragma once

#include <wx/string.h>

#include <wx/event.h>
#include <wx/socket.h>


namespace snakes {

	// Size of a game block
	#define GAME_CELL_SIZE 10
	// Number of blocks making up a full horizontal or vertical line.
	#define GAME_LINE_SIZE 60

	enum EngineType
	{
		ET_CLIENT,
		ET_HOST
	};

	struct Engine {
		virtual void ProcessEvent(wxSocketEvent& event) = 0;
		virtual void SetGameComponent(wxWindow* window) = 0;
		virtual void Destroy() = 0;
	};

	struct EngineFactory
	{
		static Engine* Create(EngineType type, const wxString& connectionString, wxEvtHandler* handler);
	};
}
