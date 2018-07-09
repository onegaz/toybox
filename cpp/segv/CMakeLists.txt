cmake_minimum_required (VERSION 3.5)
project (segv)
set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_BUILD_TYPE "Debug" CACHE
      STRING "Choose the type of build." FORCE)

add_executable(${CMAKE_PROJECT_NAME} segv.cpp)

