cmake_minimum_required (VERSION 3.5)
project (filestore)
set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Threads)
find_package(Boost REQUIRED COMPONENTS program_options filesystem system)

include_directories($ENV{HOME}/oss/cpp-driver/include)
link_directories($ENV{HOME}/oss/cpp-driver/build)

add_executable(${CMAKE_PROJECT_NAME} filestore.cpp)

target_link_libraries (${CMAKE_PROJECT_NAME} cassandra Boost::filesystem 
	Boost::program_options Boost::system
	${CMAKE_THREAD_LIBS_INIT})