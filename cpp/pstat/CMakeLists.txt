cmake_minimum_required (VERSION 3.5)
project (pstat)
set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Boost REQUIRED COMPONENTS program_options filesystem system)
find_package(Threads)

add_executable(${CMAKE_PROJECT_NAME} pstat.cpp )
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${CMAKE_PROJECT_NAME} 
	Boost::program_options 
	Boost::filesystem
	Boost::system
	${CMAKE_THREAD_LIBS_INIT})