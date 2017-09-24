cmake_minimum_required(VERSION 2.8)
project(txt2img)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

if(UNIX AND NOT APPLE)
    # for Linux, BSD, Solaris, Minix
	set(pngwriter_dir "~/oss/pngwriter") 
	set(freetype2_dir "/usr")
	link_directories("/usr/lib/x86_64-linux-gnu")
endif()

if(APPLE)
    # for MacOS X
	set(pngwriter_dir "/oss/pngwriter")
	set(freetype2_dir "/usr/local/Cellar/freetype/2.8.1")
endif()

if(WIN32)
    # for Windows operating system in general
endif()

include_directories("${pngwriter_dir}/src")
include_directories("${freetype2_dir}/include/freetype2")
link_directories(${pngwriter_dir})
link_directories(${freetype2_dir}/lib)

add_executable(${PROJECT_NAME} txt2img.cpp)

target_link_libraries (txt2img pngwriter freetype pthread boost_system boost_program_options )
