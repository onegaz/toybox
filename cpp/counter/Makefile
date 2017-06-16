all: counter

counter: main.cpp counter.hpp
	g++ -o $@ -std=c++11 -g $<

clean:
	@if [ -f counter ]; then rm counter; fi;	