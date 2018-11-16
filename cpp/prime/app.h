#pragma once
#include <wx/wx.h>
#include <memory>
#include <thread>
#include <vector>

class MyApp : public wxApp
{
public:
    bool OnInit() override;
    void Stop();
    void StartTask(int n);
    void SendUpdateEvent();
    void WorkThreadProc(int n);
    std::unique_ptr<std::thread> m_thread;
    std::vector<int> m_primes;
};

wxDECLARE_APP(MyApp);
wxDECLARE_EVENT(wxEVT_MY_EVENT, wxCommandEvent);
std::vector<int> sieve_of_sundaram(int n);
