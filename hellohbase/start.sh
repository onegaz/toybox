#!/bin/bash

#java -cp /home/onega/bin/hadoop-2.7.3/share/hadoop/common/lib/commons-logging-1.1.3.jar:/home/onega/bin/hadoop-2.7.3/share/hadoop/common/hadoop-common-2.7.3.jar:/home/onega/bin/hbase-1.3.0/lib/hbase-protocol-1.3.0.jar:/home/onega/bin/hbase-1.3.0/lib/hbase-client-1.3.0.jar:. CreateHBaseTable t3 cf3
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64

export CLASSPATH=$(~/bin/hbase-2.0.0/bin/hbase classpath)
#javap -s org.apache.hadoop.hbase.HBaseConfiguration
#javap -s org.apache.hadoop.conf.Configuration
#javap -s org.apache.hadoop.hbase.client.ConnectionFactory
#javap -s org.apache.hadoop.hbase.client.Connection | grep -A 1 "getAdmin"
#javap -s org.apache.hadoop.hbase.TableName | grep -A 1 valueOf
#javap -s org.apache.hadoop.hbase.HTableDescriptor | grep -A 1 "addFamily"
#javap -s org.apache.hadoop.hbase.HColumnDescriptor 
javap -s  org.apache.hadoop.hbase.client.Admin   | grep -A 1 createTable
exit 0


hbasecheck=$(${JAVA_HOME}/bin/jps | grep -E "[0-9]+ HMaster" | wc -l)

if [ "${hbasecheck}" -eq 0 ]; 
then
    echo "HBase is not found, please start ~/bin/hbase-2.0.0/bin/start-hbase.sh"
    exit 1
fi


${JAVA_HOME}/bin/java -cp $(~/bin/hbase-2.0.0/bin/hbase classpath):. CreateHBaseTable t2 cf