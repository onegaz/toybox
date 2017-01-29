
export HADOOP_HOME=/home/onega/bin/hadoop-2.7.3
export HADOOP_COMMON_LIB_NATIVE_DIR=$HADOOP_HOME/lib/native
export HADOOP_OPTS="-Djava.library.path=$HADOOP_HOME/lib/native"

$HADOOP_HOME/bin/yarn application -list
$HADOOP_HOME/bin/hdfs dfs -rm /output1/*
$HADOOP_HOME/bin/hdfs dfs -rmdir /output1 
$HADOOP_HOME/bin/mapred pipes -D mapred.job.name=wordcount -D mapreduce.pipes.isjavarecordreader=true -D mapreduce.pipes.isjavarecordwriter=true -input /input1/hadoop-policy.xml -output /output1 -program /bin/wordcount