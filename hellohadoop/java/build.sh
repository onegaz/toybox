#!/bin/bash
if [ ! -f $HADOOP_HOME/bin/hadoop ]; then
    echo "hadoop is not found! Please make sure hadoop is installed and HADOOP_HOME is exported."
    exit 1
fi

#HADOOP_CLASSPATH=$JAVA_HOME/lib/tools.jar $HADOOP_HOME/bin/hadoop com.sun.tools.javac.Main *.java -d $PWD
javac -cp /home/onega/bin/hadoop-2.7.3/share/hadoop/common/hadoop-common-2.7.3.jar:/home/onega/bin/hadoop-2.7.3/share/hadoop/mapreduce/hadoop-mapreduce-client-core-2.7.3.jar *.java -d $PWD

if [ $? -ne 0 ]
then
  exit 1
fi

jar cmvf MANIFEST.MF hellohadoop.jar  org/

if [ $? -ne 0 ]
then
  echo "Could not create jar file" >&2
  exit 1
fi

mkdir deps
pushd deps

if [ $? -ne 0 ]
then
  echo "Could not change to deps folder" >&2
  exit 1
fi

java -Xmx512m -jar /home/onega/bin/tattletale-1.2.0.Beta2/tattletale.jar /home/onega/github/onegaz/toybox/hellohadoop/java 

popd

exit 0

java -cp ./hellohadoop.jar:$(/home/onega/bin/hadoop-2.7.3/bin/hadoop classpath) org.myorg.hellohadoop 

#jar -tf hellohadoop.jar

if [ $? -ne 0 ]
then
  exit 1
fi

