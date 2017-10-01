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
#include <string>
#include <deque>
#include <boost/version.hpp>
#include <boost/config.hpp>
#include <boost/predef.h>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QDesktopWidget>
#include <QtCore/QStandardPaths> // /usr/local/opt/qt/include/QtCore/QStandardPaths
#include <QtGui/QKeyEvent>
#include <QtGui/QClipboard>
#include <QtCore/QSettings>
#include <QtCore/QFileInfo>

#include "text_to_img.hpp"
#include "pngwriter_text_to_img.hpp"
#include "wnd.h"
#include "imageviewer.hpp"

QString get_cfg_path()
{
    static QString app_settings_path;
    if (app_settings_path.length() < 1)
    {
        QString applicationFilePath = QCoreApplication::applicationFilePath();
        QFileInfo fi(applicationFilePath);
        QString name = fi.fileName();
        QString homedir = QStandardPaths::locate(QStandardPaths::HomeLocation, QString(),
                                                 QStandardPaths::LocateDirectory);
        if (!homedir.endsWith(QDir::separator()))
            homedir.append(QDir::separator());
        app_settings_path = homedir + QString(".") + name + QString(".ini");
    }
    return app_settings_path;
}

void MainWnd::CreateButtons()
{
    button_about = new QPushButton("About", this->centralWidget());
    QObject::connect(button_about, SIGNAL(clicked()), this, SLOT(clickedAbout()));
    button_run = new QPushButton("Run", this->centralWidget());
    QObject::connect(button_run, SIGNAL(clicked()), this, SLOT(clickedRun()));
    button_exit = new QPushButton("Exit", this->centralWidget());
    QObject::connect(button_exit, SIGNAL(clicked()), this, SLOT(clickedSlot()));
    button_tff = new QPushButton("Font File", this->centralWidget());
    QObject::connect(button_tff, SIGNAL(clicked()), this, SLOT(clickedSlot()));
    button_srcpath = new QPushButton("Source", this->centralWidget());
    QObject::connect(button_srcpath, SIGNAL(clicked()), this, SLOT(clickedSlot()));
    button_dstpath = new QPushButton("Save to", this->centralWidget());
    QObject::connect(button_dstpath, SIGNAL(clicked()), this, SLOT(clickedSlot()));
    button_append = new QPushButton("Append", this->centralWidget());
    QObject::connect(button_append, SIGNAL(clicked()), this, SLOT(clickedSlot()));
    button_copy = new QPushButton("Copy", this->centralWidget());
    QObject::connect(button_copy, SIGNAL(clicked()), this, SLOT(clickedSlot()));
    button_clear = new QPushButton("Clear", this->centralWidget());
    QObject::connect(button_clear, SIGNAL(clicked()), this, SLOT(clickedSlot()));
}

MainWnd::MainWnd(QWidget* parent, Qt::WindowFlags flags) : QMainWindow(parent, flags)
{
    this->setCentralWidget(new QWidget(this->centralWidget()));
    QGridLayout* layout = new QGridLayout();

    CreateButtons();
    QSettings settings(get_cfg_path(), QSettings::NativeFormat);

#if BOOST_OS_WINDOWS

#elif BOOST_OS_LINUX
    std::string ttf = "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc";
#elif BOOST_OS_MACOS
    std::string ttf = "/System/Library/Fonts/STHeiti Light.ttc";
#endif

    QString sText = settings.value("tff_path", ttf.c_str()).toString();
    tff_path = new QLineEdit(sText, this->centralWidget());

    sText = settings.value("src_path", "").toString();
    src_path = new QLineEdit(sText, this->centralWidget());

    sText = QString("%1/txt2img.png").arg(QDir::homePath());
    sText = settings.value("dst_path", sText).toString();
    dst_path = new QLineEdit(sText, this->centralWidget());
    sText = settings.value("m_font_size", "12").toString();
    m_font_size = new QLineEdit(sText, this->centralWidget());
    sText = settings.value("m_width", "960").toString();
    m_width = new QLineEdit(sText, this->centralWidget());
    sText = settings.value("m_bgcolor", "65535").toString();
    m_bgcolor = new QLineEdit(sText, this->centralWidget());

    QLabel* txt_label = new QLabel("Text", this->centralWidget());
    QLabel* fontsize_label = new QLabel("Font Size", this->centralWidget());
    QLabel* width_label = new QLabel("Width", this->centralWidget());
    QLabel* bgcolor_label = new QLabel("Background", this->centralWidget());

    src_text = new QPlainTextEdit(this->centralWidget());

    int rowspan = 1, colspan = 1;
    int row = 0;
    int col = 0;
    layout->addWidget(button_tff, row, col);
    col++;
    layout->addWidget(tff_path, row, col, rowspan, colspan = 6);

    row++;
    col = 0;
    layout->addWidget(button_dstpath, row, col);
    col++;
    layout->addWidget(dst_path, row, col, rowspan, colspan = 6);

    row++;
    col = 0;
    layout->addWidget(fontsize_label, row, col);
    col++;
    layout->addWidget(m_font_size, row, col, rowspan = 1, colspan = 1);
    col++;
    layout->addWidget(width_label, row, col, rowspan = 1, colspan = 1);
    col++;
    layout->addWidget(m_width, row, col, rowspan = 1, colspan = 1);
    col++;
    layout->addWidget(bgcolor_label, row, col, rowspan = 1, colspan = 1);
    col++;
    layout->addWidget(m_bgcolor, row, col, rowspan = 1, colspan = 1);

    row++;
    col = 0;
    layout->addWidget(button_srcpath, row, col);
    col++;
    layout->addWidget(src_path, row, col, rowspan = 1, colspan = 6);

    row++;
    col = 0;
    layout->addWidget(txt_label, row, col);
    col++;
    layout->addWidget(src_text, row, col, rowspan = 5, colspan = 6);
    row++;
    col = 0;
    layout->addWidget(button_append, row, col);
    row++;
    col = 0;
    layout->addWidget(button_copy, row, col);
    row++;
    col = 0;
    layout->addWidget(button_clear, row, col);
    row++;
    col = 0; // extra row for src_text
    layout->rowStretch(row);

    QHBoxLayout* lastRowLayout = new QHBoxLayout;
    lastRowLayout->addWidget(button_about);
    lastRowLayout->addWidget(button_run);
    lastRowLayout->addWidget(button_exit);

    row++;
    col = 0;
    layout->addLayout(lastRowLayout, row, col = 0, rowspan = 1, colspan = 7);

    this->centralWidget()->setLayout(layout);
    layout->setColumnStretch(6, 100);
    QSize wndsize = QDesktopWidget().availableGeometry(this).size() * 0.7;
    resize(wndsize);
}

MainWnd::~MainWnd()
{
}

void MainWnd::clickedAbout()
{
    std::ostringstream ss;
    ss << QCoreApplication::applicationFilePath().toStdString() << " ";
    ss << "pid " << QCoreApplication::applicationPid() << std::endl;
    ss << "Build with Compiler: " << BOOST_COMPILER << std::endl
       << "Platform: " << BOOST_PLATFORM << std::endl
       << "Library: " << BOOST_STDLIB << std::endl
       << "Boost " << BOOST_LIB_VERSION << std::endl
       << "QT " << std::hex << QT_VERSION << std::endl
       << "PATH=" << getenv("PATH") << std::endl
       << "LD_LIBRARY_PATH=" << getenv("LD_LIBRARY_PATH") << std::endl;
    QMessageBox::about(this, "About", ss.str().c_str());
}

void MainWnd::saveSettings()
{
    QSettings settings(get_cfg_path(), QSettings::NativeFormat);
    settings.setValue("src_path", src_path->text());
    settings.setValue("dst_path", dst_path->text());
    settings.setValue("m_width", m_width->text());
    settings.setValue("m_font_size", m_font_size->text());
    settings.setValue("tff_path", tff_path->text());
}

void MainWnd::clickedRun()
{
    int width = m_width->text().toInt();
    int fs = m_font_size->text().toInt();
    int ls = 4;
    QFileInfo check_file(src_path->text());
    if (check_file.exists() == false || check_file.isFile() == false)
    {
        if (src_text->toPlainText().isEmpty())
            return;
    }
    saveSettings();
    std::unique_ptr<pngwriter_text_to_img> t2i = std::make_unique<pngwriter_text_to_img>(
        tff_path->text().toStdString(), width, fs, ls, m_bgcolor->text().toInt(),
        src_path->text().toStdString(), dst_path->text().toStdString());

    if (src_text->toPlainText().isEmpty())
        t2i->convert(); //
    else
    {
        t2i->convert(src_text->toPlainText().toStdString());
    }
    ImageViewer imageViewer(dst_path->text());
    imageViewer.exec();
}

void MainWnd::clickedSlot()
{
    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
    if (clickedButton == button_srcpath)
    {
        QString file1Name = QFileDialog::getOpenFileName(
            this, tr("choose a text file"),
            QStandardPaths::locate(QStandardPaths::ApplicationsLocation, QString(),
                                   QStandardPaths::LocateDirectory),
            tr("*"));
        if (file1Name.length())
        {
            src_path->setText(file1Name);
            QSettings settings(get_cfg_path(), QSettings::NativeFormat);
            settings.setValue("src_path", file1Name);
        }
        return;
    }

    if (clickedButton == button_dstpath)
    {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save as png file"), "",
                                                        tr("png (*.png)"));
        if (fileName.isEmpty() == false)
            dst_path->setText(fileName);
        return;
    }

    if (clickedButton == button_tff)
    {
        QString file1Name = QFileDialog::getOpenFileName(
            this, tr("choose a font file"), "/usr/share/fonts/opentype/noto", tr("*"));
        if (file1Name.length())
        {
            tff_path->setText(file1Name);
            QSettings settings(get_cfg_path(), QSettings::NativeFormat);
            settings.setValue("tff_path", file1Name);
        }
        return;
    }

    if (clickedButton == button_exit)
    {
        close();
        return;
    }

    if (clickedButton == button_clear)
    {
        src_text->clear();
        return;
    }

    if (clickedButton == button_copy)
    {
        auto clipboard = QApplication::clipboard();
        clipboard->setText(src_text->toPlainText());
        return;
    }

    if (clickedButton == button_append)
    {
        QString cbtxt = QApplication::clipboard()->text();
        if (!cbtxt.isEmpty())
        {
            src_text->moveCursor(QTextCursor::End);
            src_text->insertPlainText(cbtxt);
            src_text->moveCursor(QTextCursor::End);
        }
        return;
    }
}

#include "moc_wnd.cpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    MainWnd w;
    w.show();
    return app.exec();
}
