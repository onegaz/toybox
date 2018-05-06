THRIFT_ROOT=$(HOME)/oss/thrift-0.11.0
JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
LDFLAGS = -lboost_thread -lboost_program_options -lthrift -lboost_system -ljvm -L $(JAVA_HOME)/jre/lib/amd64/server -L $(HOME)/oss/thrift-0.11.0/lib/cpp/.libs -L $(HOME)/bin/zookeeper-3.4.12/src/c/.libs -lzookeeper_mt
CPPFLAGS = -std=c++11 -g -pthread -I $(JAVA_HOME)/include -I $(JAVA_HOME)/include/linux -I $(HOME)/bin/zookeeper-3.4.12/src/c/include -I $(HOME)/bin/zookeeper-3.4.12/src/c/generated

all: hellohbase CreateHBaseTable.class

hellohbase: hellohbase.cpp gen-cpp/hbase_constants.cpp  gen-cpp/hbase_types.cpp  gen-cpp/THBaseService.cpp  
	g++ $(CPPFLAGS) -I gen-cpp -I $(THRIFT_ROOT)/lib/cpp/src -o $@ $^ $(LDFLAGS)

CreateHBaseTable.class: CreateHBaseTable.java
	$(JAVA_HOME)/bin/javac -cp `~/bin/hbase-2.0.0/bin/hbase classpath` -Xlint:deprecation CreateHBaseTable.java 

clean:
	@if [ -f hellohbase ] ; then rm hellohbase ; fi ;  
	@if [ -f CreateHBaseTable.class ] ; then rm CreateHBaseTable.class ; fi ;  
