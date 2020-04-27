#!/bin/bash

rm CMakeCache.txt

cmake -DCMAKE_TOOLCHAIN_FILE=~/oss/vcpkg/scripts/buildsystems/vcpkg.cmake \
	-DwxWidgets_CONFIG_EXECUTABLE=~/oss/vcpkg/buildtrees/wxwidgets/x64-osx-rel/wx-config \
	-DwxWidgets_LIB_DIR=~/oss/vcpkg/buildtrees/wxwidgets/x64-osx-rel/lib \
	-DwxWidgets_USE_STATIC=ON -DwxWidgets_USE_UNICODE=ON \
	..

cmake --build .

exit 0

BOOST_ROOT="${BOOST_ROOT:-~/Downloads/boost_1_69_0}"
BOOST_INCLUDEDIR="${BOOST_INCLUDEDIR:-~/Downloads/boost_1_69_0}"

wxWidgets_CONFIG_EXECUTABLE="${wxWidgets_CONFIG_EXECUTABLE:-~/Downloads/wxWidgets-3.1.2/bld/wx-config}"

cmake -DBOOST_INCLUDEDIR=~/Downloads/boost_1_69_0 \
-DBOOST_INCLUDEDIR=~/Downloads/boost_1_69_0 \
-DBOOST_ROOT=~/Downloads/boost_1_69_0 \
-DZLIB_ROOT=~/oss/zlib \
 -DwxWidgets_CONFIG_EXECUTABLE=~/Downloads/wxWidgets-3.1.2/bld/wx-config ..
 
cmake --build .

exit 0

cmake -DBOOST_INCLUDEDIR=~/Downloads/boost_1_69_0 \
-DBOOST_INCLUDEDIR=~/Downloads/boost_1_69_0 \
-DBOOST_ROOT=~/Downloads/boost_1_69_0 \
-DZLIB_ROOT=~/oss/zlib \
 -DwxWidgets_CONFIG_EXECUTABLE=~/Downloads/wxWidgets-3.1.2/bld/wx-config ..
 
