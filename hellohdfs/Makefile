HADOOP_HDFS_HOME ?= $(HOME)/bin/hadoop-3.1.0

LDFLAGS = -L /usr/lib/jvm/java-8-openjdk-amd64/jre/lib/amd64/server

all: hellohdfs

clean:
		@if [ -f hellohdfs ]; then rm hellohdfs; fi;

hellohdfs: hellohdfs.cpp
		$(CXX) -o  $@ -I$(HADOOP_HDFS_HOME)/include -g -std=c++11 $^ -lhdfs -ljvm -L$(HADOOP_HDFS_HOME)/lib/native $(LDFLAGS)
