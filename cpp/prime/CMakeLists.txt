cmake_minimum_required (VERSION 3.5)
project (prime)
set(CMAKE_CXX_STANDARD 14) # C++11...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")
SET (CMAKE_CXX_COMPILER  "clang++")

find_package(Threads)
find_package(Boost REQUIRED COMPONENTS program_options filesystem system timer)
find_package(wxWidgets REQUIRED core base)
include(${wxWidgets_USE_FILE})

IF(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2")    
ENDIF(CMAKE_COMPILER_IS_GNUCC)

add_executable(prime prime.cpp)
target_link_libraries(prime 
	Boost::program_options 
	Boost::timer
	Boost::system 
	${CMAKE_THREAD_LIBS_INIT})
	
add_executable(wxprime app.cpp frame.cpp)
target_link_libraries(wxprime 
	${wxWidgets_LIBRARIES}
	Boost::program_options 
	Boost::timer
	Boost::system 
	${CMAKE_THREAD_LIBS_INIT})
	
# sudo apt install libboost-timer-dev