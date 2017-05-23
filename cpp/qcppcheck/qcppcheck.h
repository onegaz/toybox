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

private:
  	QPushButton *button_cppcheck;
  	QLineEdit *cppcheck_path;
  	QLineEdit *cppcheck_options;
  	QPushButton *button_srcpath;
  	QLineEdit *cppcheck_srcpath;
  	QPlainTextEdit* cppcheck_errors;
    QPushButton *button_about;
    QPushButton *button_run;
    QPushButton *button_exit;
    std::mutex g_output_mutex;
    std::stringstream m_output;
};

#endif /* MAINWND_H_ */