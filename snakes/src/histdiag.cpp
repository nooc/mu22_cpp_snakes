#include <wx/wxprec.h>

#include "histdiag.h"


using namespace snakes;

HistoryDialog::HistoryDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer(wxVERTICAL);

	m_listBox1 = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	m_listBox1->SetMinSize(wxSize(300, 200));

	bSizer6->Add(m_listBox1, 0, wxALL, 5);

	m_button6 = new wxButton(this, wxID_OK, wxT("Close"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer6->Add(m_button6, 0, wxALL | wxEXPAND, 5);


	this->SetSizer(bSizer6);
	this->Layout();
	bSizer6->Fit(this);

	this->Centre(wxBOTH);

	// Connect Events
	//m_button6->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(HistoryDialog::handleClose), NULL, this);
}

HistoryDialog::~HistoryDialog()
{
	// Disconnect Events
	//m_button6->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(HistoryDialog::handleClose), NULL, this);

}
