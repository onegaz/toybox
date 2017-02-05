THRIFT_ROOT=/home/onega/src/thrift-0.9.3
LDFLAGS = -lboost_thread -lboost_program_options -lthrift -lboost_system -L /home/onega/bin/zookeeper-3.4.9/src/c/.libs -lzookeeper_mt
CPPFLAGS = -std=c++11 -g -pthread -I /home/onega/bin/zookeeper-3.4.9/src/c/include -I /home/onega/bin/zookeeper-3.4.9/src/c/generated

all: hellohbase CreateHBaseTable.class

hellohbase: hellohbase.cpp gen-cpp/hbase_constants.cpp  gen-cpp/hbase_types.cpp  gen-cpp/THBaseService.cpp  
	g++ $(CPPFLAGS) -I gen-cpp -I $(THRIFT_ROOT)/lib/cpp/src/thrift -o $@ $^ $(LDFLAGS)

CreateHBaseTable.class: CreateHBaseTable.java
	javac -cp `~/bin/hbase-1.3.0/bin/hbase classpath` -Xlint:deprecation CreateHBaseTable.java 

clean:
	@if [ -f hellohbase ] ; then rm hellohbase ; fi ;  
	@if [ -f CreateHBaseTable.class ] ; then rm CreateHBaseTable.class ; fi ;  
