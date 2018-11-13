#include "app.h"
#include "frame.h"
// clang-format off
wxIMPLEMENT_APP(MyApp);
// clang-format on

bool MyApp::OnInit()
{
    auto title = wxString::Format("Hello %s (pid %lu)", wxVERSION_STRING,
                                  wxGetProcessId());
    MyFrame* frame = new MyFrame(title, wxPoint(50, 50), wxSize(800, 340));
    frame->Show(true);
    return true;
}

void MyApp::Stop()
{
    if (m_thread)
    {
        m_thread->join();
        m_thread.reset();
    }
}
