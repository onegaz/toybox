CXX=/home/onega/gcc-6.3.0/bin/g++
CXX=LD_LIBRARY_PATH=/home/onega/gcc-6.3.0/lib64:$LD_LIBRARY_PATH /home/onega/bin/clang+llvm-3.9.0-x86_64-fedora23/bin/clang
CXX=g++
LDFLAGS=-L /home/onega/src/cpprestsdk/Release/Binaries -L /home/onega/gcc-6.3.0/lib64 -lcpprest -lcommon_utilities -lboost_program_options -lboost_system -lssl -lcrypto -ldl -lstdc++
LDFLAGS=-L /home/onega/src/cpprestsdk/Release/Binaries -lcpprest -lcommon_utilities -lboost_program_options -lboost_system -lssl -lcrypto -ldl -lstdc++

all: cpprestclient1

hello-cpprestsdk: hello-cpprestsdk.cpp 
	$(CXX) -o $@ -std=c++11 -I /home/onega/src/cpprestsdk/Release/include -pthread -g $^ $(LDFLAGS)
	echo LD_LIBRARY_PATH=/home/onega/src/cpprestsdk/Release/Binaries:/usr/local/lib:'$$LD_LIBRARY_PATH' ./$@

cpprestclient1: cpprestclient1.cpp 
	$(CXX) -o $@ -std=c++11 -I /home/onega/src/cpprestsdk/Release/include -pthread -g $^ $(LDFLAGS)
	echo LD_LIBRARY_PATH=/home/onega/src/cpprestsdk/Release/Binaries:/usr/local/lib:'$$LD_LIBRARY_PATH' ./$@

clean:  
	@if [ -f hello-cpprestsdk ] ; then rm hello-cpprestsdk ; fi ;
	@if [ -f cpprestclient1 ] ; then rm cpprestclient1 ; fi ;