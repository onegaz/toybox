all: hellokafka

hellokafka: hellokafka.cpp
	g++ -o $@ -g -pthread -I/home/onega/src/libkafka-asio/lib -I/home/onega/src/snappy -std=c++11 $^ -L/home/onega/src/snappy/.libs -lsnappy -lboost_program_options -lboost_system -lz

clean:
	rm hellokafka
