#pragma once

#include <wx/wxchar.h>
#include <vector>

namespace snakes {

	class HistoryManager
	{
		wxString m_file;
		std::vector<wxString> m_hist;
	public:
		HistoryManager(const wxString& fileName);
		~HistoryManager();

		void Add(const wxString& line);
		void Save();
		std::vector<wxString> GetAll() const { return m_hist; }
	};
}
