cmake_minimum_required (VERSION 3.5)
project (CuckooMap)
set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Threads)
find_package(Boost REQUIRED COMPONENTS program_options regex system)

add_executable(${CMAKE_PROJECT_NAME} custom_alloc.cpp)

if (DEFINED ENV{LIBCDS_HOME})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE 
	${LIBCDS_HOME}
	)
else()
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE 
	~/oss/libcds
	)
endif()

target_link_libraries (${CMAKE_PROJECT_NAME} 
    Boost::program_options
    Boost::regex
	${CMAKE_THREAD_LIBS_INIT})
	
# LIBCDS_HOME=~/oss/libcds cmake ..