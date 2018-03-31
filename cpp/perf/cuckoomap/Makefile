all: cuckoo_perf custom_alloc

cuckoo_perf: perf.cpp
	g++ -o $@ -pthread -m64 -std=c++11 -O2 -I ~/oss/libcds -L ~/oss/libcds-rel/bin perf.cpp -lboost_program_options -lboost_system 
	
	#-lcds
custom_alloc: custom_alloc.cpp
	g++ -o $@ -pthread -m64 -std=c++11 -g -O2 -I ~/oss/libcds -L ~/oss/libcds-rel/bin $^ -lboost_program_options -lboost_system 

clean:
	rm cuckoo_perf custom_alloc