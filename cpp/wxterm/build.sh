#!/bin/bash

rm CMakeCache.txt

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