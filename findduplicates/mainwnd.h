#include <wx/clipbrd.h>
#include <wx/gbsizer.h>
#include <wx/button.h>
#include <wx/frame.h>
#include <wx/app.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/clipbrd.h>
#include <wx/msgdlg.h>
#include <wx/listbox.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/timer.h>

class MyFrame : public wxFrame
{
    wxTextCtrl* m_output;
    wxListBox *m_lb;
    wxButton *m_addfile;
    wxButton *m_addfolder;
    wxButton *m_removeselected;
    wxButton *m_about;
    wxButton *m_start;
    wxButton *m_cancel;
    wxButton *m_exit;
    wxTimer* m_timer;
public:

    MyFrame() ;

	void OnRightClick(wxListEvent& event)
	{
	    wxMenu menu(wxT("Context Menu"));
	    	menu.Append(wxID_COPY, wxT("&Copy to clipboard"));
	    menu.Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnPopupClick), NULL, this);
	    PopupMenu(&menu, event.GetPoint());
	}
	void OnItemSelected(wxListEvent& event)
	{

	}
	void OnPopupClick(wxCommandEvent &evt)
	 {

	 }
	void OnFindDoneEvent( wxCommandEvent &evt );
	void OnExit( wxCommandEvent& event );
	void OnAbout( wxCommandEvent& event );
	void OnCancel( wxCommandEvent& event );
	void OnStart( wxCommandEvent& event );
	void OnAddFile( wxCommandEvent& event );
	void OnAddFolder( wxCommandEvent& event );
	void OnRemove( wxCommandEvent& event );
    void OnTimerTimeout(wxTimerEvent& event);
};
