cmake_minimum_required(VERSION 3.5)
project(txt2img)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

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

find_package(Qt5Widgets)
find_package(Qt5Gui)
find_package(Qt5Core)
find_package(Threads)
#include_directories("$ENV{Qt5_DIR}/include")

include_directories("${pngwriter_dir}/src")
include_directories("${freetype2_dir}/include/freetype2")
link_directories(${pngwriter_dir})
link_directories(${freetype2_dir}/lib)

add_executable(${PROJECT_NAME} txt2img.cpp wnd.cpp pngwriter_text_to_img.cpp imageviewer.cpp)

target_link_libraries (txt2img pngwriter freetype boost_system boost_program_options
	Qt5::Widgets Qt5::Core ${CMAKE_THREAD_LIBS_INIT} )

# CMAKE_PREFIX_PATH=~/Qt/5.9/gcc_64/lib/cmake cmake ~/oss/toybox/txt2img
# Qt5_DIR=~/Qt/5.9/gcc_64 CMAKE_PREFIX_PATH=~/Qt/5.9/gcc_64/lib/cmake cmake ~/oss/toybox/txt2img && make VERBOSE=1
