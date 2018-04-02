all: send_file

send_file: send_file.cpp
	g++ -o $@ -g -std=c++11 -pthread $< -l boost_program_options -l boost_system
	
clean: 
	@if [ -f send_file ]; then rm send_file; fi;