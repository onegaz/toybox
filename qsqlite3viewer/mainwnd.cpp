#include "mainwnd.h"
#include <QtGui/QMessageBox>
#include <QtGui/QKeyEvent>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QFileDialog>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <sqlite3/sqlite3.h>
#include <SQLiteCpp/SQLiteCpp.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/null.hpp>

MainWnd::MainWnd(QWidget *parent, Qt::WFlags flags) :
		QMainWindow(parent, flags) {
	this->setCentralWidget(new QWidget(this->centralWidget()));
	QGridLayout* layout = new QGridLayout();
  QHBoxLayout *firstRowLayout = new QHBoxLayout;
  table_list = new QListWidget(this);
  table_view = new QTableView(this);
	choose_db = new QPushButton("Sqlite3 DB", this->centralWidget());
	QObject::connect(choose_db, SIGNAL(clicked()),this, SLOT(clickedSlot()));

	button_about = new QPushButton("About", this->centralWidget());
	QObject::connect(button_about, SIGNAL(clicked()),this, SLOT(clickedSlot()));

  button_exit = new QPushButton("Exit", this->centralWidget());
	QObject::connect(button_exit, SIGNAL(clicked()),this, SLOT(clickedSlot()));

	db_path = new QLineEdit(this->centralWidget());

  firstRowLayout->addWidget(choose_db); firstRowLayout->addWidget(db_path);
  layout->addLayout(firstRowLayout,0,0, 1, 2);

	layout->addWidget(table_list, 1, 0);
	layout->addWidget(table_view, 1, 1);

  QHBoxLayout *lastRowLayout = new QHBoxLayout;

	lastRowLayout->addWidget(button_about);
	lastRowLayout->addWidget(button_exit);
  layout->addLayout(lastRowLayout,2,0, 1, 2);

	// table_view->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->setColumnStretch(1, 100);
	db_path->installEventFilter(this);

  connect(table_list, SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(itemClicked(QListWidgetItem*)));
	this->centralWidget()->setLayout(layout);
  table_data = new QStandardItemModel();
}

MainWnd::~MainWnd() {
}

void MainWnd::showTableList(std::deque<std::string>& tblist){
  table_list->clear();
  for(auto tbname: tblist) {
    QListWidgetItem * item1 = new QListWidgetItem(tbname.c_str());
    table_list->addItem(item1);
  }
}

void MainWnd::showTable(std::deque<std::string>& headers, std::deque<std::string>& data){
  table_data->clear();
  table_data->setRowCount(0);
  if(headers.size()<1)
  return;

  int rows = data.size()/headers.size();
  int columncnt(headers.size());
  table_data->setColumnCount(columncnt);
      qDebug() << __func__ ;
std::vector<size_t> columnsize(headers.size());
  for(int col=0; col<headers.size(); col++) {
		table_data->setHorizontalHeaderItem(col, new QStandardItem(headers[col].c_str()));
		columnsize[col] =  headers[col].length();
	}

  for(int row=0; row<rows; row++) {
    QList<QStandardItem *> rowData;
    for(int col=0; col<columncnt; col++) {
      int pos = row*columncnt+col;
      rowData << new QStandardItem(QString(data[pos].c_str()));
			columnsize[col] = std::max(columnsize[col], data[pos].length());
    }
    table_data->appendRow(rowData);
  }
  table_view->setModel(table_data);
	for (int col=0; col<columncnt; col++)
	{
	   table_view->setColumnWidth(col,std::min(columnsize[col]*12, (size_t)360)); // column not too wide
	}
  table_view->show();
}

void MainWnd::itemClicked(QListWidgetItem* item)
{
  std::cout << item->text().toStdString() << std::endl;
  std::deque<std::string> column_names, dbdatacol;
  std::string table_name = item->text().toStdString();
  try
  {
      SQLite::Database    db(db_path->text().toStdString().c_str());
      std::stringstream ss;
      ss << "SELECT * FROM " << table_name << " LIMIT 1"; // only check first 1 row to figure out column title
      SQLite::Statement   query(db, ss.str());
      while (query.executeStep())
      {
        for(int i=0; i<query.getColumnCount(); i++) {
          // getostream() << query.getColumn(i).getName() << "\t";
          if(column_names.size()==(size_t)i) {
            column_names.push_back(query.getColumn(i).getName());
          }
        }
      }
  }
  catch (std::exception& e)
  {
      std::cout << "exception: " << e.what() << std::endl;
  }
  try
  {
      SQLite::Database    db(db_path->text().toStdString().c_str());
      std::stringstream ss;
      ss << "SELECT * FROM " << table_name;
      SQLite::Statement   query(db, ss.str());
      int n=0;
      while (query.executeStep())
      {
        for(int i=0; i<query.getColumnCount(); i++) {
              dbdatacol.push_back(query.getColumn(i).getText());
        }
      }
  }
  catch (std::exception& e)
  {
      std::cout << "exception: " << e.what() << std::endl;
  }
  showTable(column_names, dbdatacol);
};

void MainWnd::clickedSlot() {
	QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
	if (clickedButton == button_about) {
    std::ostringstream ss;
    ss << QCoreApplication::applicationFilePath().toStdString() << " ";
    ss << "pid " << QCoreApplication::applicationPid() << std::endl;
    ss << "Build with g++ " << __GNUC__<<"." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << std::endl;
    QMessageBox::information(this,"About",ss.str().c_str());
		return;
	}
  if (clickedButton == button_exit) {
		close();
		return;
	}
  if (clickedButton == choose_db) {
    QString file1Name = QFileDialog::getOpenFileName(this,
       tr("Open sqlite3 database file"), getenv("HOME"), tr("*"));
       db_path->setText(file1Name);
      table_list->clear();
      table_data->clear();
      if(file1Name.length()==0)
        return;
      try
      {
          SQLite::Database    db(file1Name.toStdString().c_str());
          SQLite::Statement   query(db, "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name");
          while (query.executeStep())
          {
            table_list->addItem(query.getColumn(0).getText());
          }
      }
      catch (std::exception& e)
      {
          std::cout << "exception: " << e.what() << std::endl;
      }
    return;
  }
}
// this tool used https://github.com/SRombauts/SQLiteCpp
