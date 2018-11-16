#include "app.h"
#include "frame.h"
// clang-format off
wxIMPLEMENT_APP(MyApp);
wxDEFINE_EVENT(wxEVT_MY_EVENT, wxCommandEvent);
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

void MyApp::StartTask(int n)
{
	Stop();
	m_thread.reset(new std::thread(&MyApp::WorkThreadProc, this, n));
}

void MyApp::SendUpdateEvent()
{
	wxCommandEvent event( wxEVT_MY_EVENT );
	wxPostEvent( GetTopWindow (), event );
}

void MyApp::WorkThreadProc(int n)
{
	m_primes = sieve_of_sundaram(n);
	wxCommandEvent event( wxEVT_MY_EVENT );
	wxPostEvent( GetTopWindow (), event );
}
