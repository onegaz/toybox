cmake_minimum_required(VERSION 3.5)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Debug" CACHE
      STRING "Choose the type of build." FORCE)

find_package(Threads)
find_package(Boost REQUIRED COMPONENTS program_options filesystem regex)
#include_directories(/usr/local/Cellar/boost/1.71.0/include)

add_executable(filterlogcat filterlogcat.cpp )

target_link_libraries (filterlogcat
	Boost::program_options
	${CMAKE_THREAD_LIBS_INIT})
