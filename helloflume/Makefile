THRIFT_ROOT=/home/onega/src/thrift-0.10.0

CXX=LD_LIBRARY_PATH=/home/onega/gcc-6.3.0/lib64:${LD_LIBRARY_PATH} /home/onega/gcc-6.3.0/bin/g++
LDFLAGS=-L /home/onega/gcc-6.3.0/lib64 -L /usr/local/lib -lthrift -lboost_program_options -lboost_system -ldl

CXX=LD_LIBRARY_PATH=/home/onega/gcc-6.3.0/lib64:${LD_LIBRARY_PATH} /home/onega/bin/clang+llvm-3.9.0-x86_64-fedora23/bin/clang
LDFLAGS=-L /usr/local/lib -L /home/onega/gcc-6.3.0/lib64 -lthrift -lboost_program_options -lboost_system -ldl -lstdc++

CFLAGS=-std=c++11 -g -pthread -I gen-cpp -I $(THRIFT_ROOT)/lib/cpp/src/thrift

all: helloflume

helloflume: helloflume.cpp gen-cpp/ThriftSourceProtocol.cpp gen-cpp/flume_constants.cpp gen-cpp/flume_types.cpp
	$(CXX) -o $@ $(CFLAGS) $^ $(LDFLAGS)
	@echo LD_LIBRARY_PATH=/home/onega/gcc-6.3.0/lib64:'$$LD_LIBRARY_PATH' ./$@

gen-cpp/ThriftSourceProtocol.cpp:
	thrift --gen cpp /home/onega/bin/apache-flume-1.7.0-src/flume-ng-sdk/src/main/thrift/flume.thrift

clean:  
	@if [ -f helloflume ] ; then rm helloflume ; fi ;
