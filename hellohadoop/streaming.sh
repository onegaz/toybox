
export HADOOP_HOME=/home/onega/bin/hadoop-2.7.3
export HADOOP_COMMON_LIB_NATIVE_DIR=$HADOOP_HOME/lib/native
export HADOOP_OPTS="-Djava.library.path=$HADOOP_HOME/lib/native"

$HADOOP_HOME/bin/yarn application -list
$HADOOP_HOME/bin/hdfs dfs -rm /output1/*
$HADOOP_HOME/bin/hdfs dfs -rmdir /output1 
#$HADOOP_HOME/bin/hdfs dfs -rm /bin/hellostreamingmapper
#$HADOOP_HOME/bin/hdfs dfs -rm /bin/hellostreamingreducer
#$HADOOP_HOME/bin/hdfs dfs -put /home/onega/github/onegaz/toybox/hellohadoop/hellostreaming /bin/hellostreamingmapper
#$HADOOP_HOME/bin/hdfs dfs -put /home/onega/github/onegaz/toybox/hellohadoop/hellostreaming /bin/hellostreamingreducer

cp -p /home/onega/github/onegaz/toybox/hellohadoop/hellostreaming /home/onega/github/onegaz/toybox/hellohadoop/hellostreamingmapper
cp -p /home/onega/github/onegaz/toybox/hellohadoop/hellostreaming /home/onega/github/onegaz/toybox/hellohadoop/hellostreamingreducer

#$HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/tools/lib/hadoop-streaming-2.7.3.jar -input /input1/hadoop-policy.xml -output /output1 -mapper hdfs://localhost:9000/bin/hellostreamingmapper -reducer hdfs://localhost:9000/bin/hellostreamingreducer

$HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/tools/lib/hadoop-streaming-2.7.3.jar -input /input1/hadoop-policy.xml -output /output1 -mapper /home/onega/github/onegaz/toybox/hellohadoop/hellostreamingmapper -reducer /home/onega/github/onegaz/toybox/hellohadoop/hellostreamingreducer