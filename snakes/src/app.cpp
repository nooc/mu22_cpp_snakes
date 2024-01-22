#include <wx/wxprec.h>

#include "app.h"
#include "frame.h"

wxIMPLEMENT_APP(snakes::App);

bool snakes::App::OnInit()
{
    auto frame = new snakes::MainFrame(wxT("Snakes"), wxPoint(50, 50), wxSize(450, 340));
    frame->Show(true);
	return true;
}
