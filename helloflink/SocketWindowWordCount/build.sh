#!/bin/bash

export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
export PATH=/usr/lib/jvm/java-8-openjdk-amd64/bin:$PATH


FLINK_HOME=~/bin/flink-1.4.2

java -version

javac -cp ~/bin/flink-1.4.2/lib/flink-dist_2.11-1.4.2.jar SocketWindowWordCount.java

jar cvmf manifest.txt myclient.jar *class

#~/bin/flink-1.4.2/bin/flink run myclient.jar --hostname localhost --port 12345

echo "java -classpath ~/bin/flink-1.4.2/lib/flink-dist_2.11-1.4.2.jar:myclient.jar SocketWindowWordCount --hostname localhost --port 12345"

# http://www.java2s.com/Code/JarDownload/irclib/irclib-1.10.jar.zip
javac -d wikiedits -cp ~/bin/flink-1.4.2/lib/flink-dist_2.11-1.4.2.jar:~/bin/irclib-1.10.jar wikiedits/*.java

${FLINK_HOME}/bin/stop-local.sh
${FLINK_HOME}/bin/start-local.sh
jps

#${FLINK_HOME}/bin/flink run myclient.jar --port 9000

exit 0

cd wikiedits
jar cvmf manifest.txt wikiedits.jar wikiedits

export CLASSPATH=${FLINK_HOME}/opt/flink-metrics-prometheus-1.4.2.jar:${CLASSPATH}
export CLASSPATH=${FLINK_HOME}/opt/flink-metrics-ganglia-1.4.2.jar:${CLASSPATH}
export CLASSPATH=${FLINK_HOME}/opt/flink-metrics-statsd-1.4.2.jar:${CLASSPATH}
export CLASSPATH=${FLINK_HOME}/opt/flink-table_2.11-1.4.2.jar:${CLASSPATH}
export CLASSPATH=${FLINK_HOME}/opt/lib/flink-dist_2.11-1.4.2.jar:${CLASSPATH}
