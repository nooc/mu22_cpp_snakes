#include "history.h"
#include <fstream>

using namespace snakes;


HistoryManager::HistoryManager(const wxString& fileName): m_file(fileName)
{
	std::ifstream strm;
	strm.open(fileName.mb_str(), std::ifstream::in);
	if (strm.is_open())
	{
		std::string line;
		while (!strm.eof())
		{
			std::getline(strm, line);
			if(!line.empty()) m_hist.push_back(line);
		}
	}
}

HistoryManager::~HistoryManager()
{
}

void HistoryManager::Add(const wxString& line)
{
	m_hist.push_back(line);
}

void HistoryManager::Save()
{
	std::ofstream strm;
	strm.open(m_file.mb_str(), std::ofstream::out);
	if (strm.is_open())
	{
		for (auto i = m_hist.begin(), j = m_hist.end(); i != j; i++)
		{
			auto buf = i->mb_str();
			strm.write(buf.data(),buf.length());
			strm << "\n";
		}
	}
}

