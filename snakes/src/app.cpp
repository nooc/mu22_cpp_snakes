#include <wx/wxprec.h>
#include <wx/socket.h>

#include "app.h"
#include "frame.h"

// wxWidgets main macro
wxIMPLEMENT_APP(snakes::App);

bool snakes::App::OnInit()
{
    // initialize socket stack
    wxSocketBase::Initialize();

    // set main frame
    auto frame = new snakes::MainFrame(wxT("Snakes"), wxPoint(50, 50), wxSize(450, 340));
    frame->Show(true);
	return true;
}

int snakes::App::OnExit()
{
    // shut down socket stack
    wxSocketBase::Shutdown();

    return wxApp::OnExit();
}
