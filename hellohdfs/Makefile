CXX=/home/onega/gcc-6.3.0/bin/g++

HADOOP_HDFS_HOME ?= /home/onega/bin/hadoop-2.7.3

LDFLAGS = -L/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.60-2.b27.el7_1.x86_64/jre/lib/amd64/server

all: hellohdfs

clean:
		@if [ -f hellohdfs ]; then rm hellohdfs; fi;

hellohdfs: hellohdfs.cpp
		$(CXX) -o $@ -I$(HADOOP_HDFS_HOME)/include -std=c++11 $^ -lhdfs -ljvm -L$(HADOOP_HDFS_HOME)/lib/native $(LDFLAGS)
