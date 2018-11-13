#pragma once
#include <wx/wx.h>
#include <wx/listctrl.h>

class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

private:
    void OnHello(wxCommandEvent& event);
    void OnStart(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    enum
    {
        ID_Hello = 1
    };

    wxTextCtrl* m_max_num;
    wxButton* m_start;
    wxButton* m_exit;
    wxListCtrl* m_item_list;
    wxDECLARE_EVENT_TABLE();
};
