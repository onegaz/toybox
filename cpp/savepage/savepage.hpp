#ifndef MAINWND_H_
#define MAINWND_H_

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtCore/QTime>
#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryFile>
#include <mutex>
#include <sstream>

class MainWnd : public QMainWindow {
public:
    Q_OBJECT
public:
    MainWnd(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~MainWnd();
public slots:
    void clickedSlot();
    void customEvent(QEvent* e);
public:
    static const int UPDATE_CUSTOM_EVENT=1001;
    void output(const std::string& msg);
    bool createjs();
    void append(QPlainTextEdit* qpte, const std::string& msg);
private:
    QPushButton *button_phantomjs; // select path of phantomjs
    QLineEdit *phantomjs_path;

    QPushButton *button_output_path; // select output file path

    QPlainTextEdit* url_input;
    QPlainTextEdit* page_output;

    QPushButton *button_about;
    QPushButton *button_run;
    QPushButton *button_clear;
    QPushButton *button_exit;
    std::mutex g_output_mutex;
    std::stringstream m_output;
    QTemporaryFile savepage_js;
};

#endif /* MAINWND_H_ */
