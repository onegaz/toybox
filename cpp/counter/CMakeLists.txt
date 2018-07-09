project(counter)
cmake_minimum_required(VERSION 3.5)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_BUILD_TYPE "Debug" CACHE
      STRING "Choose the type of build." FORCE)

find_package(Boost REQUIRED COMPONENTS filesystem program_options system)
find_package(Threads)

find_package(wxWidgets COMPONENTS core base REQUIRED)
include(${wxWidgets_USE_FILE})
 
add_executable(counter main.cpp)
