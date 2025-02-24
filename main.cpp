#include <GL/glew.h>

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <vector>
#include "DpSounderCanvas.h"
#include <wx/timer.h>

class MyFrame : public wxFrame {
public:
    MyFrame() : wxFrame(nullptr, wxID_ANY, "Echogram Viewer", wxDefaultPosition, wxSize(800, 600)) {
       m_dpobj = new DpSounderCanvas(this);
       m_timer.SetOwner(this);
       Bind(wxEVT_TIMER, &MyFrame::OnTimer, this);
       m_timer.Start(1000); // Start the timer to trigger every 1000ms (1 second)
    }

    // Recursively call wxWindow::Show () on all sizer items.
    virtual void ShowItems(bool show)
    {
        std::vector<uint8_t> testData(1024, 122);
        m_dpobj->OnNewEchogram(testData);

    }

    void OnTimer(wxTimerEvent&)
    {
        ShowItems(1); // Call the function every second
    }

    wxTimer m_timer;
    DpSounderCanvas *m_dpobj;
};

// Temporary test code in OnNewEchogram()

class MyApp : public wxApp {
public:
    virtual bool OnInit() {
        m_frame = new MyFrame();
        m_frame->Show();
       
        return true;
    }

    virtual bool ProcessIdle()
    {
        // Create a timer and bind it to the event handler
       
     /*   m_frame->ShowItems(1);*/
        return wxApp::ProcessIdle();
    }

  
    MyFrame* m_frame;
};

wxIMPLEMENT_APP(MyApp);
