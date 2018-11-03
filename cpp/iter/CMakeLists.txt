cmake_minimum_required (VERSION 3.5)
project (iter)
set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Threads)

add_executable(${CMAKE_PROJECT_NAME} iter.cpp)

target_link_libraries (${CMAKE_PROJECT_NAME} 
	${CMAKE_THREAD_LIBS_INIT})
