cmake_minimum_required (VERSION 3.5)
project (hellogrpc)
set(CMAKE_CXX_STANDARD 11) 

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Threads)
find_package(Boost REQUIRED COMPONENTS program_options filesystem system)

add_custom_command(
      OUTPUT api.grpc.pb.cc  api.grpc.pb.h  api.pb.cc  api.pb.h
      COMMAND protoc
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out ${CMAKE_CURRENT_BINARY_DIR}
        -I "${CMAKE_SOURCE_DIR}"
        --plugin=protoc-gen-grpc="/usr/local/bin/grpc_cpp_plugin"
        api.proto
      DEPENDS ${CMAKE_SOURCE_DIR}/api.proto)

include_directories($ENV{HOME}/oss/grpc/include out ${CMAKE_CURRENT_BINARY_DIR})
link_directories($ENV{HOME}/oss/grpc/libs/opt)

      
add_executable(${CMAKE_PROJECT_NAME} hellogrpc.cpp 
	${CMAKE_CURRENT_BINARY_DIR}/api.pb.cc 
	${CMAKE_CURRENT_BINARY_DIR}/api.grpc.pb.cc)

target_link_libraries (${CMAKE_PROJECT_NAME} grpc++ grpc protobuf Boost::filesystem 
	Boost::program_options Boost::system
	${CMAKE_THREAD_LIBS_INIT})