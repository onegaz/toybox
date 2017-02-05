#!/bin/bash
export HADOOP_HOME=/home/onega/bin/hadoop-2.7.3
export HADOOP_COMMON_LIB_NATIVE_DIR=$HADOOP_HOME/lib/native
export HADOOP_OPTS="-Djava.library.path=$HADOOP_HOME/lib/native"

$HADOOP_HOME/bin/yarn application -list
$HADOOP_HOME/bin/hdfs dfs -rm /output1/*
$HADOOP_HOME/bin/hdfs dfs -rmdir /output1 
$HADOOP_HOME/bin/hadoop jar hellohadoop.jar org.myorg.hellohadoop /input1/hadoop-policy.xml /output1