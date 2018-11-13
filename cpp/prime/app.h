#pragma once
#include <wx/wx.h>
#include <memory>
#include <thread>

class MyApp : public wxApp
{
public:
    bool OnInit() override;
    void Stop();
    std::unique_ptr<std::thread> m_thread;
};
