/*
 * MainWnd.cpp
 *
 *  Created on: Mar 18, 2017
 *      Author: onegaz
 */
#include <spawn.h> 
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <memory>
#include <sstream>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QDesktopWidget>
#include <QtCore/QStandardPaths> // /usr/local/opt/qt/include/QtCore/QStandardPaths
#include <QtGui/QKeyEvent>
#include <QtCore/QSettings>
#include <QtCore/QFileInfo>
#include <boost/version.hpp>
#include <boost/config.hpp>
#include "qcppcheck.h"

QString get_cfg_path()
{
	static QString app_settings_path;
	if(app_settings_path.length()<1)
	{
		QString applicationFilePath = QCoreApplication::applicationFilePath();
		QFileInfo fi(applicationFilePath);
		QString name = fi.fileName();
	    QString homedir = QStandardPaths::locate(QStandardPaths::HomeLocation, QString(),
	                                                QStandardPaths::LocateDirectory);
	    if(!homedir.endsWith(QDir::separator()))
	        homedir.append(QDir::separator());
	    app_settings_path = homedir + QString(".") + name + QString(".ini");
	}
    return app_settings_path;
}

MainWnd::MainWnd(QWidget *parent, Qt::WindowFlags flags) :
		QMainWindow(parent, flags) {
	this->setCentralWidget(new QWidget(this->centralWidget()));
	QGridLayout* layout = new QGridLayout();

	button_about = new QPushButton("About", this->centralWidget());
	QObject::connect(button_about, SIGNAL(clicked()),this, SLOT(clickedSlot()));

	button_exit = new QPushButton("Exit", this->centralWidget());
	QObject::connect(button_exit, SIGNAL(clicked()),this, SLOT(clickedSlot()));

	button_cppcheck = new QPushButton("cppcheck", this->centralWidget());
	QObject::connect(button_cppcheck, SIGNAL(clicked()),this, SLOT(clickedSlot()));
	
	button_srcpath = new QPushButton("Source", this->centralWidget()); 
	QObject::connect(button_srcpath, SIGNAL(clicked()),this, SLOT(clickedSlot()));
	
	button_run = new QPushButton("Run", this->centralWidget()); 
	QObject::connect(button_run, SIGNAL(clicked()),this, SLOT(clickedSlot()));
	
    QSettings settings(get_cfg_path(), QSettings::NativeFormat);
    QString sText = settings.value("cppcheck_path", "cppcheck").toString();
	cppcheck_path = new QLineEdit(sText, this->centralWidget());

	sText = settings.value("cppcheck_srcpath", QString(getcwd(nullptr, 0))).toString();
	cppcheck_srcpath = new QLineEdit(sText, this->centralWidget());

	cppcheck_errors = new QPlainTextEdit(this->centralWidget());
	
	int rowspan=1, colspan=1; int row = 0; int col = 0;
	layout->addWidget(button_cppcheck, row, col);
	col++;
	layout->addWidget(cppcheck_path, row, col, rowspan, colspan);
	
	QLabel* cppcheck_opt_label = new QLabel("options", this->centralWidget());
	sText = settings.value("cppcheck_options", "--force").toString();
	cppcheck_options = new QLineEdit(sText, this->centralWidget());
	row++; col = 0;
	layout->addWidget(cppcheck_opt_label, row, col);
	col++;
	layout->addWidget(cppcheck_options, row, col, rowspan, colspan);
	
	row++; col = 0;
	layout->addWidget(button_srcpath, row, col);
	col++;
	layout->addWidget(cppcheck_srcpath, row, col, rowspan, colspan);
	
	QLabel* cppcheck_err_label = new QLabel("cppcheck errors", this->centralWidget());
	row++; col = 0;
	layout->addWidget(cppcheck_err_label, row, col);
	col++;
	layout->addWidget(cppcheck_errors, row, col, rowspan, colspan);
	
	row++;
	QHBoxLayout *lastRowLayout = new QHBoxLayout;
	lastRowLayout->addWidget(button_about);
	lastRowLayout->addWidget(button_run);
	lastRowLayout->addWidget(button_exit);
	layout->addLayout(lastRowLayout,row,0, 1, 2);
	
	this->centralWidget()->setLayout(layout);
	layout->setColumnStretch(1, 100);
	QSize wndsize = QDesktopWidget().availableGeometry(this).size() * 0.7;
	resize(wndsize);
}

MainWnd::~MainWnd() {
}

void MainWnd::output(const std::string& msg)
{
	{
		std::lock_guard<std::mutex> guard(g_output_mutex);
		m_output << msg;		
	}
	QApplication::postEvent(this,new QEvent(QEvent::Type(UPDATE_CUSTOM_EVENT)));
}

void runcppcheck(const std::string& cmdline, MainWnd* wnd)
{
	int exit_code=0;
	int cout_pipe[2];
	int cerr_pipe[2];
	posix_spawn_file_actions_t action;
    auto t1 = std::chrono::high_resolution_clock::now();

	if(pipe(cout_pipe) || pipe(cerr_pipe))
	{
		wnd->output("pipe returned error.\n");
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
	extern char **environ;
	if(posix_spawnp(&pid, args[0], &action, NULL, &args[0], environ) != 0)
	{
		std::stringstream ss;
		ss << __func__ << "posix_spawnp returned error " << strerror(errno) << "\n";
		wnd->output(ss.str());
		return;  	
	}
	close(cout_pipe[1]), close(cerr_pipe[1]); // close child-side of pipes
	// Read from pipes
	std::string buffer(4096,' ');
	std::vector<pollfd> plist = { {cout_pipe[0],POLLIN}, {cerr_pipe[0],POLLIN} };
	int timeout_milliseconds = 9000;
	std::string to_find_occurrences_of = "(error)";
	size_t cppcheck_error_report_count = 0;

	auto count_substr = [](const std::string& base_string, const std::string& to_find_occurrences_of)
	{
		size_t substr_count = 0;
		std::string::size_type start = 0;
		while ((start = base_string.find(to_find_occurrences_of, start)) != std::string::npos) {
		    ++substr_count;
		    start += to_find_occurrences_of.length(); // see the note
		}
		return substr_count;
	};

	while(true)
	{
		int rval = poll(&plist[0],plist.size(), timeout_milliseconds);
		int readcnt = 0;
		if ( plist[0].revents&POLLIN) 
		{
			int bytes_read = read(cout_pipe[0], &buffer[0], buffer.length());
			readcnt += bytes_read;
			std::cout << buffer.substr(0, static_cast<size_t>(bytes_read));
		}
		else if ( plist[1].revents&POLLIN ) 
		{
			int bytes_read = read(cerr_pipe[0], &buffer[0], buffer.length());
			readcnt += bytes_read;
			wnd->output(buffer.substr(0, static_cast<size_t>(bytes_read)));

			cppcheck_error_report_count += count_substr(
					buffer.substr(0, static_cast<size_t>(bytes_read)),
					to_find_occurrences_of);
		}
		pid_t wpid = waitpid(pid, &exit_code, WNOHANG);
		if (wpid == pid)
		{
		    auto t2 = std::chrono::high_resolution_clock::now();
		    auto cmd_duration = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1);

			std::stringstream ss;
			ss <<"cppcheck found " << cppcheck_error_report_count << " errors in "
					<< cmd_duration.count() << " seconds\n";
			ss <<"cppcheck exit code " << exit_code << std::endl;
			wnd->output(ss.str());		
			break;
		} else if (wpid == -1)
		{
			std::stringstream ss;
			ss <<"waitpid error when waiting for pid " << pid << " cppcheck." << std::endl;
			wnd->output(ss.str());			
			break;
		} 
		if(rval < 0)
		{
			std::stringstream ss;
			ss <<"poll error " << rval << " on pid " << pid << std::endl;
			wnd->output(ss.str());			
			break;	
		}
		if(rval == 0)
		{
			std::stringstream ss;
			ss <<"poll timeout " << rval << " on pid " << pid << std::endl;
			wnd->output(ss.str());			
		}
	} 
	std::this_thread::sleep_for(std::chrono::milliseconds(234));
	posix_spawn_file_actions_destroy(&action);    
}

void MainWnd::customEvent(QEvent* e)
{
    if(e->type() == (QEvent::Type)UPDATE_CUSTOM_EVENT)
    {	
      	std::string msg;
  		{
			std::lock_guard<std::mutex> guard(g_output_mutex);
			msg = m_output.str();
			m_output.str("");		
		}
		cppcheck_errors->moveCursor (QTextCursor::End);
		cppcheck_errors->insertPlainText (msg.c_str());
		cppcheck_errors->moveCursor (QTextCursor::End);
    }
}
	
void MainWnd::clickedSlot() {
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    if (clickedButton == button_about) {
	    std::ostringstream ss;
	    ss << QCoreApplication::applicationFilePath().toStdString() << " ";
	    ss << "pid " << QCoreApplication::applicationPid() << std::endl;
	    ss << "Build with Compiler: " << BOOST_COMPILER << std::endl
	      << "Platform: " << BOOST_PLATFORM << std::endl
	      << "Library: " << BOOST_STDLIB << std::endl
		  << "Boost " << BOOST_LIB_VERSION << std::endl
		  << "QT " << std::hex << QT_VERSION << std::endl
		  << "PATH=" << getenv("PATH") << std::endl
		  << "LD_LIBRARY_PATH=" << getenv("LD_LIBRARY_PATH") << std::endl
		  ;
		output(ss.str());
		return;
	}
	if (clickedButton == button_exit) 
	{
		close();
		return;
	} 
	
	  if (clickedButton == button_run) {
		std::ostringstream ss;
		ss 	<< cppcheck_path->text().toLatin1().data() 
			<< " " << cppcheck_options->text().toLatin1().data() 
			<< " " << cppcheck_srcpath->text().toLatin1().data();
	    QSettings settings(get_cfg_path(), QSettings::NativeFormat);
	    settings.setValue("cppcheck_options", cppcheck_options->text());
	    settings.setValue("cppcheck_srcpath", cppcheck_srcpath->text());
		std::thread t(runcppcheck, ss.str(), this);
		t.detach();
		return;
	} 
	
	if (clickedButton == button_cppcheck) 
	{
		QString file1Name = QFileDialog::getOpenFileName(this,
			tr("choose cppcheck"),
			QStandardPaths::locate(QStandardPaths::ApplicationsLocation, QString(),
								QStandardPaths::LocateDirectory),
			tr("*"));
		if (file1Name.length())
		{
			cppcheck_path->setText(file1Name);
		    QSettings settings(get_cfg_path(), QSettings::NativeFormat);
		    settings.setValue("cppcheck_path", file1Name);
		}
		return;
	} 	
	
	if (clickedButton == button_srcpath) {
       	QString file1Name =QFileDialog::getExistingDirectory(0, "Select source Directory", 
       		QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory), 
       		QFileDialog::ShowDirsOnly|QFileDialog::DontUseNativeDialog | QFileDialog::ReadOnly| QFileDialog::ShowDirsOnly);
       	cppcheck_srcpath->setText(file1Name);  
		return;
	} 	
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWnd w;
    w.show();
    return app.exec();
}

#include "moc_qcppcheck.cpp"

const char* notes=R"(
localhost:src onzhang$ brew install qt
localhost:src onzhang$ /usr/local/Cellar/qt/5.8.0_2/bin/moc MainWnd.h -o moc_MainWnd.cpp
localhost:src onzhang$ ls
MainWnd.cpp	MainWnd.h	moc_MainWnd.cpp
localhost:src onzhang$ otool -L miniqt
miniqt:
	/usr/local/opt/qt/lib/QtWidgets.framework/Versions/5/QtWidgets (compatibility version 5.8.0, current version 5.8.0)
	/usr/local/opt/qt/lib/QtGui.framework/Versions/5/QtGui (compatibility version 5.8.0, current version 5.8.0)
	/usr/local/opt/qt/lib/QtCore.framework/Versions/5/QtCore (compatibility version 5.8.0, current version 5.8.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 307.5.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1238.60.2)
)";
