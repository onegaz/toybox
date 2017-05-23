QTDIR=~/Qt/5.8/gcc_64

all: qcppcheck

moc_qcppcheck.cpp: qcppcheck.h
	$(QTDIR)/bin/moc -o $@ $<

qcppcheck: qcppcheck.cpp moc_qcppcheck.cpp
	g++ -o $@ -std=c++11 -fPIC -pthread -I $(QTDIR)/include -L $(QTDIR)/lib $< -lQt5Widgets -lQt5Gui -lQt5Core -licui18n -licuuc -licudata

clean:
	@if [ -f qcppcheck ]; then rm qcppcheck; fi
