cmake_minimum_required (VERSION 3.5)
project (findfile)
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(wxWidgets  )
include(${wxWidgets_USE_FILE})
find_package(Threads)
find_package(Boost REQUIRED COMPONENTS program_options filesystem regex system)

add_executable(${CMAKE_PROJECT_NAME} findfile.cpp)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC 
	${Boost_INCLUDE_DIR})
	
target_link_libraries (${CMAKE_PROJECT_NAME} 
	${wxWidgets_LIBRARIES}
	Boost::regex Boost::program_options Boost::filesystem Boost::system
	${CMAKE_THREAD_LIBS_INIT})