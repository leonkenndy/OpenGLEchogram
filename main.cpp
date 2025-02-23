#include <GL/glew.h>

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <vector>
#include "Echogram.hpp"


class MyFrame : public wxFrame {
public:
    MyFrame() : wxFrame(nullptr, wxID_ANY, "Echogram Viewer", wxDefaultPosition, wxSize(800, 600)) {
        new EchogramCanvas(this);
    }
};

class MyApp : public wxApp {
public:
    virtual bool OnInit() {
        MyFrame* frame = new MyFrame();
        frame->Show();
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);
