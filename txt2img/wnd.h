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
	void clickedAbout();
	void clickedRun();
private:
	void CreateButtons();
        void saveSettings();

        QPushButton *button_tff;
        QLineEdit *tff_path;
  	QPushButton *button_srcpath;
  	QLineEdit *src_path;

  	QPushButton *button_dstpath;
  	QLineEdit *dst_path;

  	QLineEdit *m_font_size;
  	QLineEdit *m_width;
  	QLineEdit *m_bgcolor;

  	QPlainTextEdit* src_text;
  	QPushButton *button_append;
  	QPushButton *button_copy;
  	QPushButton *button_clear;

    QPushButton *button_about;
    QPushButton *button_run;
    QPushButton *button_exit;
//    std::stringstream m_output;
};

#endif /* MAINWND_H_ */
