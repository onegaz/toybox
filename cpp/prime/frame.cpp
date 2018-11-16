#include "frame.h"
#include "app.h"
#include <wx/gbsizer.h>
#include <vector>

// clang-format off
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
	EVT_MENU(ID_Hello, MyFrame::OnHello)
    EVT_MENU(wxID_EXIT, MyFrame::OnExit)
	EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
wxEND_EVENT_TABLE()
// clang-format on

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size) :
    wxFrame(NULL, wxID_ANY, title, pos, size)
{
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
                     "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);
    CreateStatusBar();
    SetStatusText("Based on " wxVERSION_STRING);

    m_max_num = new wxTextCtrl(this, wxID_ANY,
			"1000000", wxDefaultPosition, wxDefaultSize);

    m_start = new wxButton(this, wxID_ANY, _T("Start"));
    m_start->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler(MyFrame::OnStart), nullptr, this);

    m_exit = new wxButton(this, wxID_ANY, _T("Exit"));
	m_exit->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler(MyFrame::OnExit), nullptr, this);

    m_item_list = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);

    for (int i=0; i<10; i++)
    {
        wxListItem col;
        col.SetId(0);
        col.SetText( wxString::Format("%d", i) );
        col.SetWidth(100);
        m_item_list->InsertColumn(m_item_list->GetColumnCount(), col);
    }

    Bind(wxEVT_MY_EVENT, &MyFrame::OnDone, this, wxID_ANY);

	wxGridBagSizer *m_fgsizer = new wxGridBagSizer;
	int row = 0;
	m_fgsizer->Add(new wxStaticText(this, wxID_ANY, "Max number"), wxGBPosition(row,0));
	m_fgsizer->Add(m_max_num, wxGBPosition(row,1), wxGBSpan(1,1));
	++row;
	m_fgsizer->Add(m_start, wxGBPosition(row,0), wxGBSpan(1,1), wxGROW);
	m_fgsizer->Add(m_exit, wxGBPosition(row,1), wxGBSpan(1,1));
	++row;
	m_fgsizer->Add(m_item_list, wxGBPosition(row,0), wxGBSpan(1,3), wxGROW);
	m_fgsizer->AddGrowableRow(row, 7);
	m_fgsizer->AddGrowableCol(2, 7);
	this->SetSizer(m_fgsizer);
}

using namespace std;
// https://en.wikipedia.org/wiki/Sieve_of_Sundaram
std::vector<int> sieve_of_sundaram(int n)
{
    vector<int> all(n + 1);
    for (int i = 1; i < n / 2; i++)
        for (int j = i; j < n / 2; j++)
        {
            int pos = i + j + 2 * i * j;
            if (pos >= 0 && pos <= n)
                all[pos] = 1;
            else
                break;
        }

    vector<int> result;

    if (n >= 2)
        result.push_back(2);

    for (int i = 1; i < n / 2; i++)
        if (all[i] == 0)
            result.push_back(2 * i + 1);
    return result;
}

void MyFrame::OnStart(wxCommandEvent& event)
{
	long val = 65536;
	m_max_num->GetValue().ToLong(&val);
	wxGetApp().StartTask(val);
}

void MyFrame::OnDone(wxCommandEvent& event)
{
    auto primes = wxGetApp().m_primes;
    auto cols = m_item_list->GetColumnCount();

    for (int n = 0; n < primes.size() / cols; n++)
    {
        wxListItem item;
        item.SetId(n);
        item.SetText(wxString::Format("%d", primes[n * cols]));

        m_item_list->InsertItem(item);

        for (int j = 1; j < cols; j++)
        {
            m_item_list->SetItem(n, j, wxString::Format("%d", primes[n * cols + j]));
        }
    }
}

void MyFrame::OnExit(wxCommandEvent& event)
{
	wxGetApp().Stop();
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a wxWidgets program", wxGetApp().GetAppDisplayName(),
                 wxOK | wxICON_INFORMATION);
}

void MyFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}
