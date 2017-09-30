cmake_minimum_required (VERSION 3.5)
project (wxterm)
set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(wxWidgets REQUIRED core base)
include(${wxWidgets_USE_FILE})
find_package(Threads)

add_executable(${CMAKE_PROJECT_NAME} wxterm.cpp)

target_link_libraries (${CMAKE_PROJECT_NAME} 
	${wxWidgets_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT})
	
# cmake -DwxWidgets_CONFIG_EXECUTABLE=~/oss/wxWidgets-3.1.0/wx-config ~/oss/toybox/cpp/wxterm
# make