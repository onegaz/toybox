#!/bin/bash

#PATH=/usr/lib/jvm/java-9-oracle/bin:$PATH
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
export PATH=/usr/lib/jvm/java-8-openjdk-amd64/bin:$PATH
FLINK_HOME=~/bin/flink-1.4.2

java -version

java -classpath ${FLINK_HOME}/lib/flink-dist_2.11-1.4.2.jar:. SocketWindowWordCount --port 12345