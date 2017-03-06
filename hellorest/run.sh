#!/bin/bash
SCRIPT=$(readlink -f $0)
SCRIPT_PATH=`dirname $SCRIPT`

make -C $SCRIPT_PATH
if [ $? -ne 0 ]; then echo "make $SCRIPT_PATH ERROR"; exit 1; fi



LD_LIBRARY_PATH=/home/onega/src/cpprestsdk/Release/Binaries:/usr/local/lib:$LD_LIBRARY_PATH $SCRIPT_PATH/cpprestclient1
#LD_LIBRARY_PATH=/home/onega/src/cpprestsdk/Release/Binaries:/usr/local/lib:$LD_LIBRARY_PATH $SCRIPT_PATH/hello-cpprestsdk

exit 0

if [ ! -e "$SCRIPT_PATH/cpprestclient1" ]; then
       make -C $SCRIPT_PATH
fi

if [ ! -e "$SCRIPT_PATH/hello-cpprestsdk" ]; then
       make -C $SCRIPT_PATH
fi