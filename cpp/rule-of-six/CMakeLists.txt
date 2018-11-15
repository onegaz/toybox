cmake_minimum_required (VERSION 3.5)
project (rule-of-six)
set(CMAKE_CXX_STANDARD 14) 
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_BUILD_TYPE Debug)

find_package(Threads)

add_executable(user-ctor user-ctor.cpp)

target_link_libraries (user-ctor  
						${CMAKE_THREAD_LIBS_INIT})