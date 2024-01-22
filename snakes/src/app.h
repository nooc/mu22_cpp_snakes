#pragma once

#include <wx/app.h>


namespace snakes {

    class App : public wxApp
    {
    public:
        virtual bool OnInit();
    };

}