all: findduplicates

CXX = ~/gcc-6.3.0/bin/g++
CXX = g++

CPPFLAGS = -I/home/onega/src/wxWidgets-3.1.0/lib/wx/include/gtk3-unicode-3.1 -I/home/onega/src/wxWidgets-3.1.0/include -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__

LDFLAGS = -L/home/onega/src/wxWidgets-3.1.0/lib -pthread   -Wl,-rpath,/home/onega/src/wxWidgets-3.1.0/lib -lwx_gtk3u_xrc-3.1 -lwx_gtk3u_html-3.1 -lwx_gtk3u_qa-3.1 -lwx_gtk3u_adv-3.1 -lwx_gtk3u_core-3.1 -lwx_baseu_xml-3.1 -lwx_baseu_net-3.1 -lwx_baseu-3.1 

findduplicates: mainwnd.cpp findduplicates.cpp digest_thread.cpp
	$(CXX) -o $@ -g -std=c++11 -pthread $(CPPFLAGS) $^ -lssl -lcrypto $(LDFLAGS) -lboost_filesystem -lboost_program_options -lboost_system
	
clean:
	rm 	findduplicates