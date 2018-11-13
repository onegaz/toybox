all: hellokafka

SNAPPY_HOME=~/oss/snappy

hellokafka: hellokafka.cpp
	g++ -o $@ -g -pthread -I ~/oss/libkafka-asio/lib -I ${SNAPPY_HOME} -std=c++11 $^ \
		${SNAPPY_HOME}/libsnappy.a -lboost_program_options -lboost_system -lz

clean:
	@if [ -f hellokafka ]; then rm hellokafka; fi;
