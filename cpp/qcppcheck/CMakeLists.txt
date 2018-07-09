cmake_minimum_required (VERSION 3.5)
project (qcppcheck)
set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_BUILD_TYPE "Debug" CACHE
      STRING "Choose the type of build." FORCE)
      
find_package(Qt5Widgets)
find_package(Qt5Gui)
find_package(Qt5Core)
find_package(Threads)

add_executable(${CMAKE_PROJECT_NAME} qcppcheck.cpp)

target_link_libraries (${CMAKE_PROJECT_NAME} 
	Qt5::Widgets Qt5::Gui Qt5::Core ${CMAKE_THREAD_LIBS_INIT})