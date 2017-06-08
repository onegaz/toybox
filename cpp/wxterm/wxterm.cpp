#include <spawn.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <wx/wx.h>
#include <wx/clipbrd.h>
#include <wx/gbsizer.h>
#include <wx/stdpaths.h>
#include <wx/dirdlg.h>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <unistd.h>
#include <fstream>
#include <thread>
#include <mutex>
#include <sstream>
#include <boost/version.hpp>
#include <boost/config.hpp>

wxDECLARE_EVENT(wxEVT_MY_EVENT, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_MY_EVENT, wxCommandEvent);

std::string get_log_timestamp()
{
	using namespace std::chrono;
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    time_t tt = std::chrono::system_clock::to_time_t(now);
    tm local_tm = *localtime(&tt);
    std::stringstream ss;
    ss << local_tm.tm_year + 1900 
		<< std::setw(2) << std::setfill('0') << local_tm.tm_mon + 1
		<< std::setw(2) << std::setfill('0') << local_tm.tm_mday;
    return ss.str();
}

class MiniWxApp : public wxApp
{
public:
	virtual bool OnInit();
	void runcmd(const std::string& cmdline);
	std::mutex gui_mutex;
	std::stringstream ss;
	void output(const char* buf)
	{
		{
			std::lock_guard<std::mutex> guard(gui_mutex);
			ss << buf;			
		}
		wxCommandEvent event( wxEVT_MY_EVENT );
		wxPostEvent( GetTopWindow (), event );
	}
	void output(const std::string& buf)
	{
		{
			std::lock_guard<std::mutex> guard(gui_mutex);
			ss << buf;
		}
		wxCommandEvent event( wxEVT_MY_EVENT );
		wxPostEvent( GetTopWindow (), event );
	}
	std::string get_output()
	{
		std::lock_guard<std::mutex> guard(gui_mutex);
		std::string msg = ss.str();
		ss.str("");
		return msg;
	}

	std::string m_cmd_history_path;
	std::string m_cmd_output_path;
};

IMPLEMENT_APP(MiniWxApp)

class MyFrame : public wxFrame
{
public:
	MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
	void OnQuit(wxCommandEvent& event){Close(true); }
	void OnAbout(wxCommandEvent& event);
	void OnRunClick(wxCommandEvent&);
	void OnClearClick(wxCommandEvent&);
	void OnReset(wxCommandEvent&);
	void OnCd(wxCommandEvent&);
	void OnListItemSelection(wxCommandEvent& event)
	{
		if (wxNOT_FOUND!=m_cmd_history->GetSelection())
			m_cmdline->SetValue(  m_cmd_history->GetString(m_cmd_history->GetSelection()));
	}
	void OnAppendResult(wxCommandEvent & evt)
	{
		m_output->AppendText(wxGetApp().get_output());
	}
	void LoadCommandHistory()
	{
		std::ifstream historyfile(wxGetApp().m_cmd_history_path.c_str());
		std::string line;
		if(historyfile.good())
		while (std::getline(historyfile, line))
		{
			m_cmd_history->Append(line);
		}
	}
	void AddCommandToHistory()
	{
		wxString cmdstr = m_cmdline->GetValue();
		m_cmd_history->Append(cmdstr);	
		
		std::ofstream historyfile(wxGetApp().m_cmd_history_path.c_str(), std::fstream::app|std::fstream::ate);
		historyfile << cmdstr.ToStdString() << std::endl;
		historyfile.close();
	}
	void AddEmptyLine()
	{
		if(m_output->GetNumberOfLines())
			m_output->AppendText("\n");
	}
	wxListBox *m_cmd_history;
	wxTextCtrl *m_cmdline;
	wxTextCtrl *m_output;
	wxTextCtrl *m_pwd;
	wxButton *m_cd;
	wxButton *m_run;
	wxButton *m_clear;
	wxButton *m_reset;
	wxButton *m_about;
	wxButton *m_exit;
};

wxString caption = _("Command History");
bool MiniWxApp::OnInit()
{
	wxString logpath = wxStandardPaths::Get().GetDocumentsDir();
	logpath += "/wxtermc-";
	logpath += get_log_timestamp();
	logpath += ".txt";
	m_cmd_history_path = logpath.ToStdString();
	
	logpath = wxStandardPaths::Get().GetDocumentsDir();
	logpath += "/wxtermo-";
	logpath += get_log_timestamp();
	logpath += ".txt";
	m_cmd_output_path=logpath.ToStdString();
	
	MyFrame *frame = new MyFrame( caption, wxDefaultPosition, wxSize(1024, 640));
	frame->Show(true);
	SetTopWindow(frame);
	return true;
}

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
: wxFrame( NULL, -1, title, pos, size )
{
	wxMenuBar *menuBar = new wxMenuBar;
	wxMenu *menuFile = new wxMenu;
	menuFile->Append( wxID_ABOUT, _("&About…") );
	menuFile->Append( wxID_EXIT, _("E&xit") );
	menuBar->Append(menuFile, _("&File") );
	SetMenuBar(menuBar);
	Connect( wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction) &MyFrame::OnQuit );
	Connect( wxID_ABOUT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction) &MyFrame::OnAbout );
	// buttons
	m_run = new wxButton(this, wxID_ANY, wxT("Run"), wxDefaultPosition,
			wxDefaultSize, 0);
	m_clear = new wxButton(this, wxID_ANY, wxT("Clear"), wxDefaultPosition,
			wxDefaultSize, 0);
	m_reset = new wxButton(this, wxID_ANY, wxT("Clean"), wxDefaultPosition,
			wxDefaultSize, 0);
	m_about = new wxButton(this, wxID_ANY, wxT("About"), wxDefaultPosition,
			wxDefaultSize, 0);
	m_exit = new wxButton(this, wxID_ANY, wxT("Exit"), wxDefaultPosition,
			wxDefaultSize, 0);
	m_cd = new wxButton(this, wxID_ANY, wxT("PWD"), wxDefaultPosition,
			wxDefaultSize, 0);
	m_run->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler(MyFrame::OnRunClick), NULL, this);
	m_clear->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler(MyFrame::OnClearClick), NULL, this);
	m_reset->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler(MyFrame::OnReset), NULL, this);
	m_about->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler(MyFrame::OnAbout), NULL, this);
	m_exit->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler(MyFrame::OnQuit), NULL, this);
	m_cd->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
				wxCommandEventHandler(MyFrame::OnCd), NULL, this);
			
	m_cmd_history = new wxListBox(this, wxID_ANY);
	m_cmd_history->Connect(wxEVT_LISTBOX_DCLICK,
			wxCommandEventHandler(MyFrame::OnListItemSelection), NULL, this);
	
	m_cmdline = new wxTextCtrl(this, wxID_ANY,
			wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_PROCESS_ENTER); // wxHSCROLL|
 	m_cmdline->SetWindowStyle(m_cmdline->GetWindowStyle() & ~wxTE_DONTWRAP | wxTE_BESTWRAP);
 	m_cmdline->Connect(wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(MyFrame::OnRunClick),NULL, this);

	m_output = new wxTextCtrl(this, wxID_ANY,
			wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxHSCROLL);
 	m_output->SetWindowStyle(m_output->GetWindowStyle() & ~wxTE_DONTWRAP | wxTE_BESTWRAP);
	m_pwd = new wxTextCtrl(this, wxID_ANY,
			wxEmptyString, wxDefaultPosition, wxDefaultSize);
	
	wxGridBagSizer *m_fgsizer = new wxGridBagSizer;
	int row = 0;
	
	m_fgsizer->Add(new wxStaticText(this, wxID_ANY, "history commands"), wxGBPosition(row,0));
	m_fgsizer->Add(m_cmd_history, wxGBPosition(row,1), wxGBSpan(2,1), wxGROW);
	row++;
	m_fgsizer->AddGrowableRow(row, 1);
	row++;
	m_fgsizer->Add(m_cd,  wxGBPosition(row,0));
	m_fgsizer->Add(m_pwd,  wxGBPosition(row,1), wxGBSpan(1,1), wxGROW);
	row++;
	m_fgsizer->Add(m_run, wxGBPosition(row,0));
	m_fgsizer->Add(m_cmdline, wxGBPosition(row,1), wxGBSpan(3,1), wxGROW);
	row++;
	m_fgsizer->Add(m_clear, wxGBPosition(row,0));
	row++;
	m_fgsizer->AddGrowableRow(row, 1);
	
	row+=1;
	m_fgsizer->Add(new wxStaticText(this, wxID_ANY, "cmd output"), wxGBPosition(row,0));
	m_fgsizer->Add(m_output, wxGBPosition(row,1), wxGBSpan(5,1), wxGROW);
	
	row++;
	m_fgsizer->Add(m_reset, wxGBPosition(row,0));
	
	row++;
	m_fgsizer->Add(m_about, wxGBPosition(row,0));
	row++;
	m_fgsizer->Add(m_exit, wxGBPosition(row,0));
	
	m_fgsizer->AddGrowableRow(row+1, 7);
	
	m_fgsizer->AddGrowableCol(1);
	
	this->SetSizer(m_fgsizer);
	Bind(wxEVT_MY_EVENT, &MyFrame::OnAppendResult, this, wxID_ANY);
	LoadCommandHistory();
}

void RunShellCommand(const std::string& cmd_in) 
{
	FILE * stream;
	const int max_buffer = 4096;
	char buffer[max_buffer];
	std::string cmd = cmd_in;
	
	cmd.append(" 2>&1");
	stream = popen(cmd.c_str(), "r");
	
	if (stream) {
		std::ofstream logfile(wxGetApp().m_cmd_output_path.c_str(), std::fstream::app|std::fstream::ate);
		logfile << cmd_in << std::endl;
		
		while (!feof(stream))
			if (fgets(buffer, max_buffer, stream) != NULL) 
			{
				logfile << buffer;
				wxGetApp().output(buffer);
			}
		pclose(stream);
		logfile << __func__ << " in thread " << wxThread::GetCurrentId() << std::endl;
		logfile << std::endl;
	}
}

void MiniWxApp::runcmd(const std::string& cmdline)
{
	int exit_code=0;
	int cout_pipe[2];
	int cerr_pipe[2];
	posix_spawn_file_actions_t action;

	if(pipe(cout_pipe) || pipe(cerr_pipe))
	{
		output("pipe returned error.\n");
		return;
	}

	posix_spawn_file_actions_init(&action);
	posix_spawn_file_actions_addclose(&action, cout_pipe[0]);
	posix_spawn_file_actions_addclose(&action, cerr_pipe[0]);
	posix_spawn_file_actions_adddup2(&action, cout_pipe[1], 1);
	posix_spawn_file_actions_adddup2(&action, cerr_pipe[1], 2);

	posix_spawn_file_actions_addclose(&action, cout_pipe[1]);
	posix_spawn_file_actions_addclose(&action, cerr_pipe[1]);

	std::string command = cmdline;
	std::string argsmem[] = {"sh","-c"}; // allows non-const access to literals
	char * args[] = {&argsmem[0][0],&argsmem[1][0],&command[0],nullptr};
	pid_t pid=0;

	if(posix_spawnp(&pid, args[0], &action, NULL, &args[0], NULL) != 0)
	{
		std::stringstream ss;
		ss << __func__ << "posix_spawnp returned error " << strerror(errno) << "\n";
		output(ss.str());
		return;
	}
	close(cout_pipe[1]), close(cerr_pipe[1]); // close child-side of pipes
	// Read from pipes
	std::string buffer(4096,' ');
	std::vector<pollfd> plist = { {cout_pipe[0],POLLIN}, {cerr_pipe[0],POLLIN} };
	int timeout_milliseconds = 5000;
	while(true)
	{
		int rval = poll(&plist[0],plist.size(), timeout_milliseconds);
		int readcnt = 0;
		if ( plist[0].revents&POLLIN)
		{
			int bytes_read = read(cout_pipe[0], &buffer[0], buffer.length());
			readcnt += bytes_read;
			std::cout << buffer.substr(0, static_cast<size_t>(bytes_read));
			output(buffer.substr(0, static_cast<size_t>(bytes_read)));
		}
		else if ( plist[1].revents&POLLIN )
		{
			int bytes_read = read(cerr_pipe[0], &buffer[0], buffer.length());
			readcnt += bytes_read;
			output(buffer.substr(0, static_cast<size_t>(bytes_read)));
		}
		pid_t wpid = waitpid(pid, &exit_code, WNOHANG);
		if (wpid == pid)
		{
			std::stringstream ss;
			ss <<"cppcheck exit code " << exit_code << std::endl;
			output(ss.str());
			break;
		} else if (wpid == -1)
		{
			std::stringstream ss;
			ss <<"waitpid error when waiting for pid " << pid << " cppcheck." << std::endl;
			output(ss.str());
			break;
		}
		if(rval < 0)
		{
			std::stringstream ss;
			ss <<"poll error " << rval << " on pid " << pid << std::endl;
			output(ss.str());
			break;
		}
		if(rval == 0)
		{
			std::stringstream ss;
			ss <<"poll timeout error " << rval << " on pid " << pid << std::endl;
			output(ss.str());
			break;
		}
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(234));
	posix_spawn_file_actions_destroy(&action);
}

void MyFrame::OnRunClick(wxCommandEvent&)
{
	if(m_cmdline->GetNumberOfLines()<1)
		return;
	
	AddCommandToHistory();
	wxString cmdstr = m_cmdline->GetValue();
	AddEmptyLine();
	std::thread t(&MiniWxApp::runcmd, &wxGetApp(), cmdstr.ToStdString());
	t.detach();
}

void MyFrame::OnClearClick(wxCommandEvent&)
{
	m_cmdline->SetValue(wxEmptyString);
}

void MyFrame::OnReset(wxCommandEvent&)
{
	m_output->SetValue(wxEmptyString);
}

void MyFrame::OnCd(wxCommandEvent&)
{
	wxDirDialog dialog(this, wxT("change working directory"), getcwd(NULL, 0), wxDD_NEW_DIR_BUTTON);
	if (dialog.ShowModal() == wxID_OK)
	{
	     wxString path = dialog.GetPath();
	     m_pwd->SetValue(path);
	     int ret = chdir(path.ToStdString().c_str());
	     if (ret)
	     {
	    	 path.Append(" failed to change");
	    	 m_pwd->SetValue(path);
	     }
	}
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	std::stringstream ss;
	ss  << wxStandardPaths::Get().GetExecutablePath() << " pid " << getpid() << std::endl
		<< "Command history file: " << wxGetApp().m_cmd_output_path << std::endl
		<< "Command output file: " << wxGetApp().m_cmd_output_path << std::endl
		<< "Compiler: " << BOOST_COMPILER << std::endl
		<< "Platform: " << BOOST_PLATFORM << std::endl
		<< "Library: " << BOOST_STDLIB << std::endl
		<< "Boost " << BOOST_LIB_VERSION << std::endl;
	
	std::string body = R"(on macOS Sierra Version 10.12.5
	g++ -o miniwx -std=c++11 $(/usr/local/opt/wxwidgets/bin/wx-config --cppflags --libs) miniwx.cpp
	localhost:wxwnd $ otool -L miniwx
	miniwx:
		/System/Library/Frameworks/IOKit.framework/Versions/A/IOKit (compatibility version 1.0.0, current version 275.0.0)
		/System/Library/Frameworks/Carbon.framework/Versions/A/Carbon (compatibility version 2.0.0, current version 157.0.0)
		/System/Library/Frameworks/Cocoa.framework/Versions/A/Cocoa (compatibility version 1.0.0, current version 22.0.0)
		/System/Library/Frameworks/AudioToolbox.framework/Versions/A/AudioToolbox (compatibility version 1.0.0, current version 492.0.0)
		/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1238.60.2)
		/System/Library/Frameworks/OpenGL.framework/Versions/A/OpenGL (compatibility version 1.0.0, current version 1.0.0)
		/usr/local/opt/wxmac/lib/libwx_osx_cocoau_xrc-3.0.dylib (compatibility version 3.0.0, current version 3.0.0)
		/usr/local/opt/wxmac/lib/libwx_osx_cocoau_webview-3.0.dylib (compatibility version 3.0.0, current version 3.0.0)
		/usr/local/opt/wxmac/lib/libwx_osx_cocoau_html-3.0.dylib (compatibility version 3.0.0, current version 3.0.0)
		/usr/local/opt/wxmac/lib/libwx_osx_cocoau_qa-3.0.dylib (compatibility version 3.0.0, current version 3.0.0)
		/usr/local/opt/wxmac/lib/libwx_osx_cocoau_adv-3.0.dylib (compatibility version 3.0.0, current version 3.0.0)
		/usr/local/opt/wxmac/lib/libwx_osx_cocoau_core-3.0.dylib (compatibility version 3.0.0, current version 3.0.0)
		/usr/local/opt/wxmac/lib/libwx_baseu_xml-3.0.dylib (compatibility version 3.0.0, current version 3.0.0)
		/usr/local/opt/wxmac/lib/libwx_baseu_net-3.0.dylib (compatibility version 3.0.0, current version 3.0.0)
		/usr/local/opt/wxmac/lib/libwx_baseu-3.0.dylib (compatibility version 3.0.0, current version 3.0.0)
		/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 307.5.0)
	)";
	ss << body;
	body = ss.str();
	AddEmptyLine();
	m_output->AppendText(body);
//	wxMessageBox( body, caption, wxOK|wxICON_INFORMATION, this );
}
