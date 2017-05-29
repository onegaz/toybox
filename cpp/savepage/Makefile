QTDIR=~/Qt/5.8/gcc_64

all: savepage

savepage: savepage.cpp moc_savepage.cpp
	g++ -o $@ -std=c++11 -fPIC -pthread -I $(QTDIR)/include -L $(QTDIR)/lib $< -lQt5Widgets -lQt5Gui -lQt5Core -licui18n -licuuc -licudata

moc_savepage.cpp: savepage.hpp
	$(QTDIR)/bin/moc -o $@ $<

clean:
	@if [ -f savepage ]; then rm savepage; fi
