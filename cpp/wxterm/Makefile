CXX=g++

all: wxterm

wxterm: wxterm.cpp
	$(CXX) -o $@ -std=c++11 `~/oss/wxWidgets-3.1.0/wx-config --cppflags` $^ `~/oss/wxWidgets-3.1.0/wx-config --libs`
	
clean:
	@if [ -f wxterm ]; then rm wxterm; fi;	
