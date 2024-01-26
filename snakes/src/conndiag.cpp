#include <wx/wxprec.h>

#include "conndiag.h"

using namespace snakes;


ConnectionDialog::ConnectionDialog( wxWindow* parent, EngineType mode, wxConfigBase& config )
	: wxDialog( parent, wxID_ANY, mode == ET_CLIENT? wxT("Connecting") : wxT("Hosting"))
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer(
		new wxStaticBox( this, wxID_ANY, mode == ET_CLIENT ? wxT("Connect to address:") : wxT("Bind to address:")),
		wxVERTICAL );
	
	m_textCtrl1 = new wxTextCtrl( sbSizer3->GetStaticBox(), wxID_ANY,
		config.Read(mode == ET_CLIENT ? "LastClientStrng" : "LastHostStrng", mode == ET_CLIENT ? wxT("127.0.0.1:7173") : wxT("0.0.0.0:7173")),
		wxDefaultPosition, wxSize( 200,-1 ), wxTE_CENTER );
	#ifdef __WXGTK__
	if ( !m_textCtrl1->HasFlag( wxTE_MULTILINE ) )
	{
	m_textCtrl1->SetMaxLength( 21 );
	}
	#else
	m_textCtrl1->SetMaxLength( 21 );
	#endif
	sbSizer3->Add( m_textCtrl1, 0, wxALL|wxEXPAND, 5 );


	bSizer3->Add( sbSizer3, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxHORIZONTAL );

	m_button3 = new wxButton( this, wxID_OK, mode == ET_CLIENT ? wxT("Join Game"): wxT("Start Hosting"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer4->Add( m_button3, 1, wxALL, 5 );

	m_button4 = new wxButton( this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer4->Add( m_button4, 1, wxALL, 5 );


	bSizer3->Add( bSizer4, 1, wxEXPAND, 5 );


	this->SetSizer( bSizer3 );
	this->Layout();
	bSizer3->Fit( this );

	this->Centre( wxBOTH );
}

ConnectionDialog::~ConnectionDialog()
{
}

wxString ConnectionDialog::GetValue() {
	return m_textCtrl1->GetValue();
}