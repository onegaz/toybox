project(wx-vcpkg-lab)
cmake_minimum_required(VERSION 3.5)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)

find_package(wxwidgets)
include(${wxWidgets_USE_FILE})


add_executable(wx-vcpkg-lab wx-vcpkg-lab.cpp )
	
target_link_libraries (wx-vcpkg-lab
	${wxWidgets_LIBRARIES}
)
