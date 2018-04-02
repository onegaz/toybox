#!/bin/bash

export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
export PATH=/usr/lib/jvm/java-8-openjdk-amd64/bin:$PATH
FLINK_HOME=~/bin/flink-1.4.2

java -version

#java -classpath ${FLINK_HOME}/lib/flink-dist_2.11-1.4.2.jar:~/.m2/repository/org/schwering/irclib/1.10/irclib-1.10.jar:target/wikiedits-flink-1.0.jar wikiedits.WikipediaAnalysis

${FLINK_HOME}/bin/flink run target/wikiedits-flink-1.0.jar

exit 0