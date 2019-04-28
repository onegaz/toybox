
#include <wx/wx.h>
#include <wx/gbsizer.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <regex>
#include <numeric>
#include <iostream>
#include <thread>
#include <mutex>

// clang-format off
wxDECLARE_EVENT(wxEVT_MY_EVENT, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_MY_EVENT, wxCommandEvent);
// clang-format on

//#pragma comment( linker, "/entry:\"mainCRTStartup\"" )
class MyApp : public wxApp
{
public:
    bool OnInit() override;
    std::vector<std::string> m_dir;
	bool m_case_sensitive = false;
	std::vector<std::string> m_plain_patterns;
	std::vector<std::string> m_regex_patterns;
	std::vector<std::regex>  m_regexs;
	boost::program_options::variables_map vm;

	std::vector<std::string> match_list;

	wxString GetStartupText();
	bool SearchFile(std::string filepath);
	void GetFilePathList(std::string dir, std::vector<std::string>& files);
	void SearchTask();
	std::thread m_search_thread;

	std::mutex gui_mutex;
	std::stringstream ss;
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
};

void MyApp::SearchTask()
{
	for (const auto& dir_path: m_dir)
	{
		std::vector<std::string> files;
		GetFilePathList(dir_path, files);
		for (const auto& file_path: files)
		{
			if (SearchFile(file_path))
			{
				std::cout << file_path << std::endl;
				output(file_path + "\n");
			}
		}
	}
	std::cout << "Done\n";
}

void MyApp::GetFilePathList(std::string dir_path, std::vector<std::string>& files)
{
    if(!boost::filesystem::exists(dir_path) || !boost::filesystem::is_directory(dir_path))
    	return;

    boost::filesystem::recursive_directory_iterator it(dir_path);
    boost::filesystem::recursive_directory_iterator endit;
    while (it != endit)
    {
        if(boost::filesystem::is_regular_file(*it))
            files.push_back(it->path().string());
        ++it;
    }
}

bool MyApp::SearchFile(std::string filepath)
{
	if (m_plain_patterns.empty() && m_regex_patterns.empty())
		return true;

	std::vector<int> plain_matches(m_plain_patterns.size());
	std::vector<int> regex_matches(m_regex_patterns.size());
	int plain_match_count = 0;
	int regex_match_count = 0;

	std::ifstream input( filepath.c_str() );
	for( std::string line; getline( input, line ); )
	{
		if (plain_match_count < m_plain_patterns.size())
		{
			for (int i=0; i<m_plain_patterns.size(); ++i)
				if (!plain_matches[i])
					plain_matches[i] |= line.find(m_plain_patterns[i]) != std::string::npos;
			plain_match_count = std::accumulate(plain_matches.begin(), plain_matches.end(), 0);
		}

		if (regex_match_count < regex_matches.size())
		{
			for (int i=0; i<m_regexs.size(); ++i)
				if (!regex_matches[i])
					regex_matches[i] |= std::regex_search(line, m_regexs[i]);
			regex_match_count = std::accumulate(regex_matches.begin(), regex_matches.end(), 0);
		}

		if (plain_match_count == m_plain_patterns.size() && regex_match_count == regex_matches.size())
			break;
	}

	return std::all_of(plain_matches.begin(), plain_matches.end(), [](int i){return i>0;});
}

class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

private:
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    wxDECLARE_EVENT_TABLE();
    enum
    {
        ID_Hello = 1
    };
    wxTextCtrl *m_cmd_output;
	void OnAppendResult(wxCommandEvent & evt);
};

// clang-format off
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
	EVT_MENU(ID_Hello, MyFrame::OnHello)
    EVT_MENU(wxID_EXIT, MyFrame::OnExit)
	EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
wxEND_EVENT_TABLE()
wxDECLARE_APP(MyApp);
wxIMPLEMENT_APP(MyApp);
// clang-format on

void MyFrame::OnAppendResult(wxCommandEvent & evt)
{
	m_cmd_output->AppendText(wxGetApp().get_output());
}

wxString MyApp::GetStartupText()
{
	if (argc==1 || argv[1][0]=='-')
		return wxEmptyString;
	wxString txt;
	for (int i=1; i<argc; i++)
	{
		txt.append(argv[i]);
		txt.append("\n");
	}
	return txt;
}

bool MyApp::OnInit()
{
	boost::program_options::options_description desc("Allowed options");
	if (argc>1 && argv[1][0]=='-')
	{
		desc.add_options()("help,h", "produce help message")
		("cui", "command line interface")
		("dir", boost::program_options::value<decltype(m_dir)>(&m_dir), "directories to search")
		("patterns", boost::program_options::value<decltype(m_plain_patterns)>(&m_plain_patterns), "plain text patterns")
		("regex_patterns", boost::program_options::value<decltype(m_regex_patterns)>(&m_regex_patterns), "regex patterns")
		("case", "case sensitive")
		  ;
		boost::program_options::store(
				boost::program_options::parse_command_line(argc, argv.operator char**(), desc), vm);
		boost::program_options::notify(vm);

		for (const auto& regex_text: m_regex_patterns)
		{
			if (vm.count("case"))
				m_regexs.emplace_back(regex_text);
			else
				m_regexs.emplace_back(regex_text, std::regex_constants::icase);
		}
	}

	if(vm.count("help")) {
		std::cout << desc << std::endl;
		return false;
	}

	m_search_thread = std::thread(&MyApp::SearchTask, this);

	if (vm.count("cui"))
		return false;

    auto title = wxString::Format("Hello %s (pid %lu)", wxVERSION_STRING,
                                  wxGetProcessId());
    MyFrame* frame = new MyFrame(title, wxPoint(50, 50), wxSize(450, 340));

    frame->Show(true);
    return true;
}

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
    SetStatusText("Welcome to wxWidgets!");

    m_cmd_output = new wxTextCtrl(this, wxID_ANY, wxGetApp().GetStartupText(), wxDefaultPosition,
                              wxDefaultSize, wxTE_MULTILINE | wxHSCROLL);
	wxGridBagSizer *m_fgsizer = new wxGridBagSizer;
	int row = 0;
	m_fgsizer->Add(m_cmd_output, wxGBPosition(row,0), wxGBSpan(1,1), wxGROW);
	m_fgsizer->AddGrowableRow(row);
	m_fgsizer->AddGrowableCol(0);
	this->SetSizer(m_fgsizer);
	Bind(wxEVT_MY_EVENT, &MyFrame::OnAppendResult, this, wxID_ANY);
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("Find file containing string patterns", "About findfile",
                 wxOK | wxICON_INFORMATION);
}

void MyFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}
