#!/bin/bash

#java -cp /home/onega/bin/hadoop-2.7.3/share/hadoop/common/lib/commons-logging-1.1.3.jar:/home/onega/bin/hadoop-2.7.3/share/hadoop/common/hadoop-common-2.7.3.jar:/home/onega/bin/hbase-1.3.0/lib/hbase-protocol-1.3.0.jar:/home/onega/bin/hbase-1.3.0/lib/hbase-client-1.3.0.jar:. CreateHBaseTable t3 cf3

hbasecheck=$(jps | grep -E "[0-9]+ HMaster" | wc -l)

if [ "${hbasecheck}" -eq 0 ]; 
then
    echo "HBase is not found, please start ~/bin/hbase-1.3.0/bin/start-hbase.sh"
    exit 1
fi


java -cp $(~/bin/hbase-1.3.0/bin/hbase classpath):. CreateHBaseTable t2 cf