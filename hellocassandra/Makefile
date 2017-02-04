THRIFT_ROOT=/home/onega/src/thrift-0.9.3
CPPFLAGS = -std=c++11 -I gen-cpp -I $(THRIFT_ROOT)/lib/cpp/src/thrift
LDFLAGS = -lthrift 
all: hellocassandra

hellocassandra: hellocassandra.cpp gen-cpp/cassandra_types.cpp gen-cpp/cassandra_constants.cpp gen-cpp/Cassandra.cpp
	g++ -o $@ -g $(CPPFLAGS) $^ $(LDFLAGS)

clean:
	@if [ -f hellocassandra ] ; then rm hellocassandra ; fi ;  

