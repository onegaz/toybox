cmake_minimum_required (VERSION 2.8)
project (prime)
set(CMAKE_CXX_STANDARD 11) # C++11...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")
SET (CMAKE_CXX_COMPILER  "clang++")

IF(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2")    
ENDIF(CMAKE_COMPILER_IS_GNUCC)

add_executable(prime prime.cpp)
target_link_libraries(prime boost_program_options.dll boost_timer)