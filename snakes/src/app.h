#pragma once

#include <wx/app.h>


namespace snakes {

    class App : public wxApp
    {
    public:
        bool OnInit();
        int OnExit();
    };

}