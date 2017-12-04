cmake_minimum_required (VERSION 3.5)
project (f2i)
set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_BUILD_TYPE Debug)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(Threads)

add_executable(${CMAKE_PROJECT_NAME} f2i.cpp )
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${CMAKE_PROJECT_NAME} 
	Boost::program_options 
	${CMAKE_THREAD_LIBS_INIT})