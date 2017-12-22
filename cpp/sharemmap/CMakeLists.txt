cmake_minimum_required (VERSION 3.5)
project (sharemmap)
set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Boost REQUIRED COMPONENTS program_options iostreams system)
find_package(Threads)

add_executable(${CMAKE_PROJECT_NAME} sharemmap.cpp )
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${CMAKE_PROJECT_NAME} 
	Boost::program_options 
	Boost::iostreams
	Boost::system
	${CMAKE_THREAD_LIBS_INIT})