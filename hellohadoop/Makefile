CXX=/home/onega/gcc-6.3.0/bin/g++
CXX=g++

HADOOP_INSTALL ?= /home/onega/bin/hadoop-2.7.3

CPPFLAGS = -I$(HADOOP_INSTALL)/include -pthread -std=c++11

all: wordcount hellostreaming

clean:
	@if [ -f wordcount ]; then rm wordcount; fi;
	@if [ -f hellostreaming ]; then rm hellostreaming; fi;

wordcount: hellohadoop.cpp
	$(CXX) -o $@ $(CPPFLAGS) hellohadoop.cpp -lhadooppipes -lhadooputils -pthread -lcrypto -L/home/onega/bin/hadoop-2.7.3/lib/native
    #$(CXX)  -o $@ $(CPPFLAGS)  $^ -lhadooppipes -lhadooputils  -L$(HADOOP_INSTALL)/lib/native
hellostreaming: hellostreaming.cpp
	$(CXX) -o $@ $(CPPFLAGS) $^  
