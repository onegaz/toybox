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
#include <QtCore/QTextStream>
#include <QtGui/QKeyEvent>
#include <boost/version.hpp>
#include <boost/config.hpp>
#include "savepage.hpp"

QString app_settings_path = "~/.savepage.ini";

const char* savepagejs=R"phantomjs(
var system = require('system');
var page   = require('webpage').create();
var fs = require('fs');

var url    = system.args[1]; // "http://www.google.com"
var savefile = system.args[2];
    page.onLoadFinished = function() {
      console.log("page load finished");
      fs.write(savefile, page.content, 'w');
      phantom.exit();
    };

page.open(url, function() {
  page.evaluate(function() {
  });
});
)phantomjs";

MainWnd::MainWnd(QWidget *parent, Qt::WindowFlags flags) :
        QMainWindow(parent, flags) {
    this->setCentralWidget(new QWidget(this->centralWidget()));
    QGridLayout* layout = new QGridLayout();

    button_about = new QPushButton("About", this->centralWidget());
    QObject::connect(button_about, SIGNAL(clicked()),this, SLOT(clickedSlot()));

    button_exit = new QPushButton("Exit", this->centralWidget());
    QObject::connect(button_exit, SIGNAL(clicked()),this, SLOT(clickedSlot()));

    button_phantomjs = new QPushButton("phantomjs", this->centralWidget());
    QObject::connect(button_phantomjs, SIGNAL(clicked()),this, SLOT(clickedSlot()));

    button_clear = new QPushButton("Clear", this->centralWidget());
    QObject::connect(button_clear, SIGNAL(clicked()),this, SLOT(clickedSlot()));

    button_run = new QPushButton("Run", this->centralWidget());
    QObject::connect(button_run, SIGNAL(clicked()),this, SLOT(clickedSlot()));

    button_output_path = new QPushButton("Save as...", this->centralWidget());
    QObject::connect(button_output_path, SIGNAL(clicked()),this, SLOT(clickedSlot()));

    QString homedir = QStandardPaths::locate(QStandardPaths::HomeLocation, QString(),
                                                QStandardPaths::LocateDirectory);
    if(!homedir.endsWith(QDir::separator()))
        homedir.append(QDir::separator());
    app_settings_path = homedir +".savepage.ini";std::cout<<app_settings_path.toStdString()<<std::endl;
    QSettings settings(app_settings_path, QSettings::NativeFormat);
    QString sText = settings.value("phantomjs_path", "phantomjs").toString();

    phantomjs_path = new QLineEdit(sText, this->centralWidget());

    url_input = new QPlainTextEdit(this->centralWidget());
    page_output = new QPlainTextEdit(this->centralWidget());

    int rowspan=1, colspan=1; int row = 0; int col = 0;
    layout->addWidget(button_phantomjs, row, col);
    col++;
    layout->addWidget(phantomjs_path, row, col, rowspan, colspan);

    QLabel* url_label = new QLabel("URL", this->centralWidget());
    row++; col = 0;
    layout->addWidget(url_label, row, col);
    col++;
//    rowspan = 2;
    layout->addWidget(url_input, row, col, rowspan, colspan);
//    row++;

    row++; col = 0; rowspan = 1;
    layout->addWidget(button_output_path, row, col);
    col++;
//    rowspan = 2;
    layout->addWidget(page_output, row, col, rowspan, colspan);
//    row++; rowspan = 1;

    row++;
    QHBoxLayout *lastRowLayout = new QHBoxLayout;
    lastRowLayout->addWidget(button_about);
    lastRowLayout->addWidget(button_run);
    lastRowLayout->addWidget(button_clear);
    lastRowLayout->addWidget(button_exit);
    layout->addLayout(lastRowLayout,row,0, 1, 2);

    this->centralWidget()->setLayout(layout);
    layout->setColumnStretch(1, 100);

    QSize wndsize = QDesktopWidget().availableGeometry(this).size() * 0.7;
    resize(wndsize);
}

bool MainWnd::createjs()
{
    if (savepage_js.open()) {
        QTextStream ts(&savepage_js);
        ts << savepagejs;
        return true;
    }
    return false;
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

void runcommand(const std::string& cmdline, MainWnd* wnd)
{
    int exit_code=0;
    int cout_pipe[2];
    int cerr_pipe[2];
    posix_spawn_file_actions_t action;

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

    if(posix_spawnp(&pid, args[0], &action, NULL, &args[0], NULL) != 0)
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
    int timeout_milliseconds = 20000;
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
        }
        pid_t wpid = waitpid(pid, &exit_code, WNOHANG);
        if (wpid == pid)
        {
            std::stringstream ss;
            ss <<"command exit code " << exit_code << std::endl;
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
        if(rval > 0 && readcnt <= 0)
        {
            std::stringstream ss;
            ss <<"poll return " << rval << " on pid " << pid <<", " << readcnt << " bytes read" << std::endl;
            wnd->output(ss.str());
             break;
        }
        if(rval == 0)
        {
            std::stringstream ss;
            ss <<"poll timeout error " << rval << " on pid " << pid << std::endl;
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
        append(page_output, msg);
    }
}

void MainWnd::clickedSlot() {
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    if (clickedButton == button_about) {
        std::ostringstream ss;
        ss << QCoreApplication::applicationFilePath().toStdString() << " ";
        ss << "pid " << QCoreApplication::applicationPid() << std::endl
           << "current directory: " << getcwd(nullptr, 0) << std::endl
          << "Build with Compiler: " << BOOST_COMPILER << std::endl
          << "Platform: " << BOOST_PLATFORM << std::endl
          << "Library: " << BOOST_STDLIB << std::endl
          << "Boost " << BOOST_LIB_VERSION << std::endl
          << "QT " << std::hex << QT_VERSION << std::endl
          << "save web page source via http://phantomjs.org/ " << std::endl;
        page_output->clear();
        page_output->appendPlainText(ss.str().c_str());
        return;
    }
    if (clickedButton == button_exit)
    {
        close();
        return;
    }
    if (clickedButton == button_clear)
    {
        url_input->clear();
        page_output->clear();
        return;
    }
    if (clickedButton == button_run) {
        append(page_output, "\n");
          std::ostringstream ss;
          ss    << phantomjs_path->text().toLatin1().data()
                << " " << savepage_js.fileName().toLatin1().data()
                << " \"" << url_input->toPlainText().toStdString() <<"\""
                << " " << page_output->toPlainText().toStdString()
              ;
          std::cout << ss.str() << std::endl;
          std::thread t(runcommand, ss.str(), this);
          t.detach();
        return;
    }

    if (clickedButton == button_phantomjs)
    {
        QString file1Name =QFileDialog::getOpenFileName(this,
            tr("phantomjs"),
            QStandardPaths::locate(QStandardPaths::ApplicationsLocation, QString(),
            QStandardPaths::LocateDirectory),
            tr("All files (*)"));
        if (file1Name.length())
        {
            phantomjs_path->setText(file1Name);
            QSettings settings(app_settings_path, QSettings::NativeFormat);
            settings.setValue("phantomjs_path", file1Name);
        }
        return;
    }
    if (clickedButton == button_output_path)
    {
        QString file1Name = QFileDialog::getExistingDirectory(this,
                                tr("Choose Or Create Directory"),
                                QStandardPaths::locate(QStandardPaths::HomeLocation, QString(),
                                            QStandardPaths::LocateDirectory),
                                QFileDialog::DontResolveSymlinks);
        if (file1Name.length())
        {
            page_output->document()->setPlainText(file1Name);
            QSettings settings(app_settings_path, QSettings::NativeFormat);
            settings.setValue("output_path", file1Name);
        }
        return;
    }
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWnd w;
    if(!w.createjs())
    {
        std::cerr << "failed to create temporary file\n";
        return 1;
    }
    w.show();
    return app.exec();
}

#include "moc_savepage.cpp"

const char* notes=R"(
localhost:src onzhang$ brew install qt
localhost:src onzhang$ (cd build/; cmake .. && make)
/usr/local/opt/qt/include/QtCore/QSettings
/usr/local/opt/qt/lib/QtWidgets.framework/Versions/5/QtWidgets (compatibility version 5.8.0, current version 5.8.0)
/usr/local/opt/qt/lib/QtGui.framework/Versions/5/QtGui (compatibility version 5.8.0, current version 5.8.0)
/usr/local/opt/qt/lib/QtCore.framework/Versions/5/QtCore (compatibility version 5.8.0, current version 5.8.0)
/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 307.5.0)
/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1238.60.2)
)";

void MainWnd::append(QPlainTextEdit* qpte, const std::string& msg)
{
    qpte->moveCursor (QTextCursor::End);
    qpte->insertPlainText (msg.c_str());
    qpte->moveCursor (QTextCursor::End);
}
