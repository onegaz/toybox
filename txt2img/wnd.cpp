/*
 * wnd.cpp
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
#include "wnd.h"

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
	button_run = new QPushButton("Run", this->centralWidget());
	QObject::connect(button_run, SIGNAL(clicked()),this, SLOT(clickedSlot()));
	button_exit = new QPushButton("Exit", this->centralWidget());
	QObject::connect(button_exit, SIGNAL(clicked()),this, SLOT(clickedSlot()));


	button_tff = new QPushButton("Font File", this->centralWidget());
	QObject::connect(button_tff, SIGNAL(clicked()),this, SLOT(clickedSlot()));

	button_srcpath = new QPushButton("Source", this->centralWidget());
	QObject::connect(button_srcpath, SIGNAL(clicked()),this, SLOT(clickedSlot()));
	button_dstpath = new QPushButton("Save to", this->centralWidget());
	QObject::connect(button_dstpath, SIGNAL(clicked()),this, SLOT(clickedSlot()));


    QSettings settings(get_cfg_path(), QSettings::NativeFormat);
    QString sText = settings.value("font_path", "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc").toString();
    tff_path = new QLineEdit(sText, this->centralWidget());

	sText = settings.value("src_path", QString(getcwd(nullptr, 0))).toString();
	src_path = new QLineEdit(sText, this->centralWidget());
	sText = settings.value("dst_path", QString(getcwd(nullptr, 0))).toString();
	dst_path = new QLineEdit(sText, this->centralWidget());
	sText = settings.value("m_font_size", QString(getcwd(nullptr, 0))).toString();
	m_font_size = new QLineEdit(sText, this->centralWidget());
	sText = settings.value("m_width", QString(getcwd(nullptr, 0))).toString();
	m_width = new QLineEdit(sText, this->centralWidget());
	sText = settings.value("m_bgcolor", QString(getcwd(nullptr, 0))).toString();
	m_bgcolor = new QLineEdit(sText, this->centralWidget());

	QLabel* txt_label = new QLabel("Text", this->centralWidget());
	QLabel* fontsize_label = new QLabel("Font Size", this->centralWidget());
	QLabel* width_label = new QLabel("Width", this->centralWidget());
	QLabel* bgcolor_label = new QLabel("Background", this->centralWidget());

	src_text = new QPlainTextEdit(this->centralWidget());

	int rowspan=1, colspan=1; int row = 0; int col = 0;
	layout->addWidget(button_tff, row, col);
	col++;
	layout->addWidget(tff_path, row, col, rowspan, colspan=6);

	row++; col = 0;
	layout->addWidget(button_dstpath, row, col);
	col++;
	layout->addWidget(dst_path, row, col, rowspan, colspan=6);

	row++; col = 0;
	layout->addWidget(fontsize_label, row, col);
	col++;
	layout->addWidget(m_font_size, row, col, rowspan=1, colspan=1);
	col++;
	layout->addWidget(width_label, row, col, rowspan=1, colspan=1);
	col++;
	layout->addWidget(m_width, row, col, rowspan=1, colspan=1);
	col++;
	layout->addWidget(bgcolor_label, row, col, rowspan=1, colspan=1);
	col++;
	layout->addWidget(m_bgcolor, row, col, rowspan=1, colspan=1);

	row++; col = 0;
	layout->addWidget(button_srcpath, row, col);
	col++;
	layout->addWidget(src_path, row, col, rowspan=1, colspan=6);

	row++; col = 0;
	layout->addWidget(txt_label, row, col);
	col++;
	layout->addWidget(src_text, row, col, rowspan=1, colspan=6);

	QHBoxLayout *lastRowLayout = new QHBoxLayout;
	lastRowLayout->addWidget(button_about);
	lastRowLayout->addWidget(button_run);
	lastRowLayout->addWidget(button_exit);

	row++; col = 0;
	layout->addLayout(lastRowLayout,row,0, 1, 7);

	this->centralWidget()->setLayout(layout);
	layout->setColumnStretch(6, 100);
	QSize wndsize = QDesktopWidget().availableGeometry(this).size() * 0.7;
	resize(wndsize);
}

MainWnd::~MainWnd() {
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
		QMessageBox::about(this, "About", ss.str().c_str());
		return;
	}
	if (clickedButton == button_exit)
	{
		close();
		return;
	}
}
#include "moc_wnd.cpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWnd w;
    w.show();
    return app.exec();
}
