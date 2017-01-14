all: qsqlite3viewer

mainwnd_moc.cpp: mainwnd.h
	/usr/lib64/qt4/bin/moc $< -o $@


qsqlite3viewer:qsqlite3viewer.cpp mainwnd.cpp mainwnd_moc.cpp
	g++ -o qsqlite3viewer -g -pthread -std=c++11 -I/home/onega/src/SQLiteCpp -I/home/onega/src/SQLiteCpp/include  $^ /home/onega/src/SQLiteCpp/Debug/libSQLiteCpp.a /home/onega/src/SQLiteCpp/Debug/sqlite3/libsqlite3.a /usr/local/lib/libboost_locale.a -licui18n -licuuc -lQtCore -lQtGui -lboost_system -ldl


clean:
	rm qsqlite3viewer
