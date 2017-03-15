CXX=/home/onega/gcc-6.3.0/bin/g++
#CXX=g++
all: hellobtree

hellobtree: hellobtree.cpp
	$(CXX) -o $@ -g -pthread -I cpp-btree -std=c++11 -O2 -fmax-errors=8 -Wall -Wl,-rpath=/home/onega/gcc-6.3.0/lib64 $^

clean:
	@if [ -f hellobtree ] ; then rm hellobtree ; fi ;
