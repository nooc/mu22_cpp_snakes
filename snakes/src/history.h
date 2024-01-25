#pragma once

#include <wx/wxchar.h>
#include <vector>

namespace snakes {

	class HistoryManager
	{
		std::vector<wxString> m_hist;
	public:
		HistoryManager(const wxString& fileName);
		~HistoryManager();

		void Add(const wxString& line);
		void Save();
	};
}
