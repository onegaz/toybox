CXX=/home/onega/gcc-6.3.0/bin/g++
#CXX=LD_LIBRARY_PATH=/home/onega/gcc-6.3.0/lib64 /home/onega/bin/clang+llvm-3.9.0-x86_64-fedora23/bin/clang
CPPFLAGS=-std=c++14 -g -fmax-errors=8
LDFLAGS=-lstdc++
all: cpptrie cpptrie-asan

cpptrie: cpptrie.cpp
	$(CXX) -o $@ $(CPPFLAGS)  -Wall -Wl,-rpath=/home/onega/gcc-6.3.0/lib64 $^ $(LDFLAGS)
cpptrie-asan: cpptrie.cpp
	$(CXX) -o $@ $(CPPFLAGS) -fsanitize=address -Wall -Wl,-rpath=/home/onega/gcc-6.3.0/lib64 $^ $(LDFLAGS)
clean:  
	@if [ -f cpptrie ] ; then rm cpptrie ; fi ;
	@if [ -f cpptrie-asan ] ; then rm cpptrie-asan ; fi ;