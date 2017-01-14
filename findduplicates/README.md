this tool find which files are duplicated by looking at their SHA1 digest.
Files with the same SHA1 digest are grouped together. Most likely they are duplicated.

[onega@localhost findduplicates]$ ~/src/wxWidgets-3.1.0/wx-config --cppflags --libs
-I/home/onega/src/wxWidgets-3.1.0/lib/wx/include/gtk3-unicode-3.1 -I/home/onega/src/wxWidgets-3.1.0/include -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__
-L/home/onega/src/wxWidgets-3.1.0/lib -pthread   -Wl,-rpath,/home/onega/src/wxWidgets-3.1.0/lib -lwx_gtk3u_xrc-3.1 -lwx_gtk3u_html-3.1 -lwx_gtk3u_qa-3.1 -lwx_gtk3u_adv-3.1 -lwx_gtk3u_core-3.1 -lwx_baseu_xml-3.1 -lwx_baseu_net-3.1 -lwx_baseu-3.1 
[onega@localhost findduplicates]$ 

15:15:24 **** Incremental Build of configuration Default for project findduplicates ****
make all 
g++ -o findduplicates -g -std=c++11 -pthread -I/home/onega/src/wxWidgets-3.1.0/lib/wx/include/gtk3-unicode-3.1 -I/home/onega/src/wxWidgets-3.1.0/include -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__ mainwnd.cpp findduplicates.cpp digest_thread.cpp -lssl -lcrypto -L/home/onega/src/wxWidgets-3.1.0/lib -pthread   -Wl,-rpath,/home/onega/src/wxWidgets-3.1.0/lib -lwx_gtk3u_xrc-3.1 -lwx_gtk3u_html-3.1 -lwx_gtk3u_qa-3.1 -lwx_gtk3u_adv-3.1 -lwx_gtk3u_core-3.1 -lwx_baseu_xml-3.1 -lwx_baseu_net-3.1 -lwx_baseu-3.1  -lboost_filesystem -lboost_program_options -lboost_system

15:15:35 Build Finished (took 10s.193ms)