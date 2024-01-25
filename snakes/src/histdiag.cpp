#include <wx/wxprec.h>

#include "histdiag.h"


using namespace snakes;

HistoryDialog::HistoryDialog(wxWindow* parent, HistoryManager* history)
	: wxDialog(parent, wxID_ANY, wxT("Past Games"))
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer(wxVERTICAL);

	m_listBox1 = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	m_listBox1->SetMinSize(wxSize(300, 200));

	bSizer6->Add(m_listBox1, 0, wxALL, 5);

	m_button6 = new wxButton(this, wxID_OK, wxT("Close"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer6->Add(m_button6, 0, wxALL | wxEXPAND, 5);

	//m_listBox1.

	this->SetSizer(bSizer6);
	this->Layout();
	bSizer6->Fit(this);

	this->Centre(wxBOTH);
}

