
#ifndef MAINWND_H_
#define MAINWND_H_
#include <QtGui/QMainWindow>
#include <QtGui/QGridLayout>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QApplication>
#include <QtGui/QListWidget>
#include <QtGui/QTableView>
#include <QtGui/QStandardItemModel>
#include <QtCore/QTime>
#include <QtCore/QDebug>
#include <deque>
#include <string>
class MainWnd : public QMainWindow
{
    Q_OBJECT
  public:
    MainWnd(QWidget *parent = 0, Qt::WFlags flags = 0);
    virtual ~MainWnd();
    void showTableList(std::deque<std::string> &tblist);
    void showTable(std::deque<std::string> &headers, std::deque<std::string> &data);
  public slots:
    void clickedSlot();
    void itemClicked(QListWidgetItem *item);

  private:
    QLineEdit *db_path;
    QPushButton *choose_db;
    QPushButton *button_about;
    QPushButton *button_exit;
    QListWidget *table_list;
    QTableView *table_view;
    QStandardItemModel *table_data;
};

#endif
/* MAINWND_H_ */
