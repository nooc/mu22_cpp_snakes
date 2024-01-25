#include "history.h"
#include <fstream>

using namespace snakes;


HistoryManager::HistoryManager(const wxString& fileName)
{
	/*
	std::ifstream strm;
	strm.open(fileName, std::ifstream::in);
	if (strm.is_open())
	{
		std::string line;
		while (!strm.eof())
		{
			std::getline(strm, line);
			if(!line.empty()) m_hist.push_back(line);
		}
	}*/
}

HistoryManager::~HistoryManager()
{
}

void HistoryManager::Add(const wxString& line)
{
}

void HistoryManager::Save()
{
}
