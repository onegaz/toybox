#!/bin/bash

export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
export PATH=/usr/lib/jvm/java-8-openjdk-amd64/bin:$PATH

java -version

~/bin/apache-maven-3.5.3/bin/mvn clean package

echo "delete all messages"
~/bin/kafka_2.12-1.1.0/bin/kafka-delete-records.sh --bootstrap-server localhost:9092 --offset-json-file ~/oss/toybox/hellokafka/javaclient/offset.json

~/bin/apache-maven-3.5.3/bin/mvn exec:java -Dexec.mainClass="javaclient.MainApp"
