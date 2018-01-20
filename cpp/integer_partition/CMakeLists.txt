cmake_minimum_required (VERSION 3.5)
project (integer_partition)

set(CMAKE_CXX_STANDARD 14) 

find_package(Boost REQUIRED COMPONENTS program_options system)

add_executable(${CMAKE_PROJECT_NAME} integer_partition.cpp)

target_link_libraries (${PROJECT_NAME} Boost::program_options)