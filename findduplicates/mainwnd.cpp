#include <wx/frame.h>
#include <wx/app.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/clipbrd.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/dirdlg.h>
#include <wx/aboutdlg.h>
#include <unordered_set>
#include <unordered_map>
#include <sstream>
#include "mainwnd.h"
#include "findduplicates.h"

MyFrame::MyFrame() : wxFrame(NULL, wxID_ANY, wxString::Format(wxT("Find duplicated files PID %i built at %s %s"),getpid(), __DATE__, __TIME__), wxPoint(50,50), wxSize(1200,600))
{
	wxGridBagSizer *gbsizer=new wxGridBagSizer();
	wxStaticText* input_label = new wxStaticText(this, wxID_ANY, _T("Find duplicated files in the following list of directories or files"));
	wxStaticText* output_label = new wxStaticText(this, wxID_ANY, _T("Result:"));
	m_output = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
			wxDefaultSize, wxTE_MULTILINE|wxVSCROLL|wxSIMPLE_BORDER );
	wxArrayString dummy;
	m_lb = new wxListBox(this, __LINE__, wxDefaultPosition, wxDefaultSize, dummy, wxLB_MULTIPLE );
	m_addfile = new wxButton(this, wxID_ANY, _T("Add File"));
	m_addfolder = new wxButton(this, wxID_ANY, _T("Add Folder"));
	m_removeselected = new wxButton(this, wxID_ANY, _T("Remove"));
	m_about = new wxButton(this, wxID_ANY, _T("About"));
	m_start = new wxButton(this, wxID_ANY, _T("Start"));
	m_cancel = new wxButton(this, wxID_ANY, _T("Cancel"));
	m_exit = new wxButton(this, wxID_ANY, _T("Exit"));
	m_timer = new wxTimer(this, 100);
	m_timer->SetOwner(this);
	m_addfile->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler( MyFrame::OnAddFile), NULL, this);
	m_addfolder->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler( MyFrame::OnAddFolder), NULL, this);
	m_removeselected->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler( MyFrame::OnRemove), NULL, this);
	m_about->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler( MyFrame::OnAbout), NULL, this);
	m_start->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler( MyFrame::OnStart), NULL, this);
	m_cancel->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler( MyFrame::OnCancel), NULL, this);
	m_exit->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
					wxCommandEventHandler( MyFrame::OnExit), NULL, this);
	int col = 0;
	int row = 0;
	gbsizer->Add(input_label, wxGBPosition(row, col), wxGBSpan(1, 4), wxEXPAND | wxRIGHT);
	row++;
	gbsizer->Add(m_addfile, wxGBPosition(row, col));
	col++;
	gbsizer->Add(m_lb, wxGBPosition(row, col), wxGBSpan(4, 3), wxEXPAND | wxRIGHT);
	row++; col=0;
	gbsizer->Add(m_addfolder, wxGBPosition(row, col));
	row++; col=0;
	gbsizer->Add(m_removeselected, wxGBPosition(row, col));
	row++; col=0;
	gbsizer->AddGrowableRow(row,1);
	row++; col=0;
	gbsizer->Add(output_label, wxGBPosition(row, col), wxGBSpan(1, 4), wxEXPAND | wxRIGHT);
	row++; col=0;
	gbsizer->Add(m_output, wxGBPosition(row, col), wxGBSpan(1, 4), wxEXPAND | wxRIGHT);
	gbsizer->AddGrowableRow(row,3);
	row++; col=0;
	gbsizer->Add(m_about, wxGBPosition(row, col));
	col++;
	gbsizer->Add(m_start, wxGBPosition(row, col));
	col++;
	gbsizer->Add(m_cancel, wxGBPosition(row, col));
	col++;
	gbsizer->Add(m_exit, wxGBPosition(row, col));
	gbsizer->AddGrowableCol(col);
	SetSizer(gbsizer);
	gbsizer->SetSizeHints(this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MyFrame::OnFindDoneEvent, this, 0);
	Connect(m_timer->GetId(),wxEVT_TIMER,wxTimerEventHandler( MyFrame::OnTimerTimeout ), NULL, this );
}

void MyFrame::OnExit( wxCommandEvent& event )
{
	wxGetApp().Cancel();
	if (event.GetEventObject()==m_exit)
	{
		Destroy();
		wxGetApp().ExitMainLoop();
	}
}

void MyFrame::OnAbout(wxCommandEvent& event) {
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName("FindDuplicates");
    aboutInfo.SetVersion("1.0");
    wxString description;
    std::stringstream ss;
    ss << "wxWidgets-based application that find duplicated files from selected files/folders.\n";
    ss << "It used the following open source libraries: boost, wxWidgets-3.1.0, crypto etc." << std::endl;
    ss << "Build with g++ "<< __GNUC__<<"." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << std::endl;
    description = ss.str();
    aboutInfo.SetDescription(description);
    aboutInfo.SetCopyright("(C) 2016-2017");
    aboutInfo.SetWebSite("https://github.com/onegaz");
    aboutInfo.AddDeveloper("OnegaZ");
    wxAboutBox(aboutInfo);
}

void MyFrame::OnCancel(wxCommandEvent& event) {
	m_timer->Stop();
	wxGetApp().Cancel();
}

void MyFrame::OnStart(wxCommandEvent& event) {
	wxGetApp().Cancel();
	m_timer->Start(2000);
	// check digest of all files, find out which files are duplicated.
	std::unordered_set<std::string>& inputpaths=wxGetApp().inputpaths;
	inputpaths.clear();
	for(int i=0; i<m_lb->GetCount(); i++) {
		std::string pathstr = m_lb->GetString(i).ToStdString();
		if(inputpaths.count(pathstr))
			continue;
		inputpaths.insert(pathstr);
	}
	// process in a thread.
	wxGetApp().m_workthread = std::thread(find_duplicates_by_digest, std::ref( inputpaths) );
}

void MyFrame::OnAddFile(wxCommandEvent& event) {
	wxFileDialog* OpenDialog = new wxFileDialog(
		this, _("Choose a file to open"), wxEmptyString, wxEmptyString,
		_("Any file (*)|*"),
		wxFD_OPEN, wxDefaultPosition);
	if (OpenDialog->ShowModal() == wxID_OK) // if the user click "Open" instead of "Cancel"
	{
		m_lb->Append(OpenDialog->GetPath());
	}
	OpenDialog->Destroy();
}

void MyFrame::OnAddFolder(wxCommandEvent& event) {
	wxDirDialog dlg(this, "Choose input directory", "",
	                wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (dlg.ShowModal()==wxID_OK) {
		m_lb->Append(dlg.GetPath());
	}
}

int compare_int(int *a, int *b)
{
  if (*a > *b) return 1;
  else if (*a < *b) return -1;
  else return 0;
}

void MyFrame::OnFindDoneEvent(wxCommandEvent& evt) {
	m_timer->Stop();
	m_output->SetValue(wxGetApp().duplicatedfiles); // m_output->SetValue(evt.GetString());
}

void MyFrame::OnRemove(wxCommandEvent& event) {
	  wxArrayInt selections;
	  selections.Sort(compare_int);
	  int count = m_lb->GetSelections(selections);
	  for(int i=0; i<count; i++) {
		  m_lb->Delete(selections[count-1-i]);
	  }
	  m_lb->SetSelection(wxNOT_FOUND);
}

void MyFrame::OnTimerTimeout(wxTimerEvent& event)
{
	wxString msg;
	msg = msg.Format("Processed %zu files\n", wxGetApp().total_file_cnt);
	m_output->AppendText(msg);
};
