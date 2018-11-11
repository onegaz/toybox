cmake_minimum_required (VERSION 3.5)
project (findduplicates)
set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(wxWidgets REQUIRED core base adv)
include(${wxWidgets_USE_FILE})

find_package(Threads)
find_package(Boost REQUIRED COMPONENTS program_options filesystem system)

include_directories($ENV{HOME}/oss/cpp-driver/include)
link_directories($ENV{HOME}/oss/cpp-driver/build)

add_executable(${CMAKE_PROJECT_NAME} digest_thread.cpp  findduplicates.cpp  mainwnd.cpp)

target_link_libraries (${CMAKE_PROJECT_NAME} 
	${wxWidgets_LIBRARIES}
	Boost::filesystem 
	Boost::program_options Boost::system
	ssl crypto
	${CMAKE_THREAD_LIBS_INIT})