[onega@localhost src]$ /home/onega/workspace/flume/start-flume.sh
Info: Including Hadoop libraries found via (/home/onega/bin/hadoop-2.7.3/bin/hadoop) for HDFS access
Info: Including Hive libraries found via () for Hive access
+ exec /usr/lib/jvm/java/bin/java -Xmx20m -Dflume.root.logger=DEBUG,console -Dorg.apache.flume.log.printconfig=true -Dorg.apache.flume.log.rawdata=true -cp 'conf:/home/onega/bin/apache-flume-1.7.0-bin/lib/*:/home/onega/bin/hadoop-2.7.3/etc/hadoop:/home/onega/bin/hadoop-2.7.3/share/hadoop/common/lib/*:/home/onega/bin/hadoop-2.7.3/share/hadoop/common/*:/home/onega/bin/hadoop-2.7.3/share/hadoop/hdfs:/home/onega/bin/hadoop-2.7.3/share/hadoop/hdfs/lib/*:/home/onega/bin/hadoop-2.7.3/share/hadoop/hdfs/*:/home/onega/bin/hadoop-2.7.3/share/hadoop/yarn/lib/*:/home/onega/bin/hadoop-2.7.3/share/hadoop/yarn/*:/home/onega/bin/hadoop-2.7.3/share/hadoop/mapreduce/lib/*:/home/onega/bin/hadoop-2.7.3/share/hadoop/mapreduce/*:/home/onega/bin/hadoop-2.7.3/contrib/capacity-scheduler/*.jar:/lib/*' -Djava.library.path=:/home/onega/bin/hadoop-2.7.3/lib/native:/usr/lib64:/home/onega/src/snappy/.libs org.apache.flume.node.Application --conf-file /home/onega/workspace/flume/helloflume.properties --name a1
17/03/05 15:48:01 INFO node.PollingPropertiesFileConfigurationProvider: Configuration provider starting
17/03/05 15:48:01 INFO node.PollingPropertiesFileConfigurationProvider: Reloading configuration file:/home/onega/workspace/flume/helloflume.properties
17/03/05 15:48:01 INFO conf.FlumeConfiguration: Added sinks: k1 Agent: a1
17/03/05 15:48:01 INFO conf.FlumeConfiguration: Processing:k1
17/03/05 15:48:01 INFO conf.FlumeConfiguration: Processing:k1
17/03/05 15:48:01 INFO conf.FlumeConfiguration: Processing:loggerSink
17/03/05 15:48:01 INFO conf.FlumeConfiguration: Processing:loggerSink
17/03/05 15:48:01 INFO conf.FlumeConfiguration: Added sinks: loggerSink Agent: agent
17/03/05 15:48:01 INFO conf.FlumeConfiguration: Post-validation flume configuration contains configuration for agents: [a1, agent]
17/03/05 15:48:01 INFO node.AbstractConfigurationProvider: Creating channels
17/03/05 15:48:01 INFO channel.DefaultChannelFactory: Creating instance of channel c1 type memory
17/03/05 15:48:01 INFO node.AbstractConfigurationProvider: Created channel c1
17/03/05 15:48:01 INFO source.DefaultSourceFactory: Creating instance of source r1, type thrift
17/03/05 15:48:01 INFO source.ThriftSource: Configuring thrift source.
17/03/05 15:48:01 INFO sink.DefaultSinkFactory: Creating instance of sink: k1, type: logger
17/03/05 15:48:01 INFO node.AbstractConfigurationProvider: Channel c1 connected to [r1, k1]
17/03/05 15:48:01 INFO node.Application: Starting new configuration:{ sourceRunners:{r1=EventDrivenSourceRunner: { source:org.apache.flume.source.ThriftSource{name:r1,state:IDLE} }} sinkRunners:{k1=SinkRunner: { policy:org.apache.flume.sink.DefaultSinkProcessor@4d97f7eb counterGroup:{ name:null counters:{} } }} channels:{c1=org.apache.flume.channel.MemoryChannel{name: c1}} }
17/03/05 15:48:01 INFO node.Application: Starting Channel c1
17/03/05 15:48:01 INFO node.Application: Waiting for channel: c1 to start. Sleeping for 500 ms
17/03/05 15:48:01 INFO instrumentation.MonitoredCounterGroup: Monitored counter group for type: CHANNEL, name: c1: Successfully registered new MBean.
17/03/05 15:48:01 INFO instrumentation.MonitoredCounterGroup: Component type: CHANNEL, name: c1 started
17/03/05 15:48:02 INFO node.Application: Starting Sink k1
17/03/05 15:48:02 INFO node.Application: Starting Source r1
17/03/05 15:48:02 INFO source.ThriftSource: Starting thrift source
17/03/05 15:48:02 INFO source.ThriftSource: Using TCompactProtocol
17/03/05 15:48:03 INFO instrumentation.MonitoredCounterGroup: Monitored counter group for type: SOURCE, name: r1: Successfully registered new MBean.
17/03/05 15:48:03 INFO instrumentation.MonitoredCounterGroup: Component type: SOURCE, name: r1 started
17/03/05 15:48:03 INFO source.ThriftSource: Started Thrift source.

[onega@localhost bin]$ jps
24200 Jps
24057 Application

[onega@localhost bin]$ netstat -tulpn
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
Active Internet connections (only servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name    
tcp        0      0 127.0.0.1:25            0.0.0.0:*               LISTEN      -                   
tcp        0      0 0.0.0.0:33049           0.0.0.0:*               LISTEN      -                   
tcp        0      0 0.0.0.0:111             0.0.0.0:*               LISTEN      -                   
tcp        0      0 0.0.0.0:22              0.0.0.0:*               LISTEN      -                   
tcp        0      0 127.0.0.1:631           0.0.0.0:*               LISTEN      -                   
tcp        0      0 127.0.0.1:5432          0.0.0.0:*               LISTEN      -                   
tcp6       0      0 ::1:25                  :::*                    LISTEN      -                   
tcp6       0      0 127.0.0.1:44444         :::*                    LISTEN      24057/java          
tcp6       0      0 :::35880                :::*                    LISTEN      -                   
tcp6       0      0 :::111                  :::*                    LISTEN      -                   
tcp6       0      0 :::22                   :::*                    LISTEN      -                   
tcp6       0      0 ::1:631                 :::*                    LISTEN      -                   
tcp6       0      0 ::1:5432                :::*                    LISTEN      -                   
udp        0      0 0.0.0.0:55337           0.0.0.0:*                           -                   
udp        0      0 0.0.0.0:68              0.0.0.0:*                           -                   
udp        0      0 0.0.0.0:111             0.0.0.0:*                           -                   
udp        0      0 0.0.0.0:123             0.0.0.0:*                           -                   
udp        0      0 127.0.0.1:323           0.0.0.0:*                           -                   
udp        0      0 0.0.0.0:49818           0.0.0.0:*                           -                   
udp        0      0 127.0.0.1:703           0.0.0.0:*                           -                   
udp        0      0 0.0.0.0:734             0.0.0.0:*                           -                   
udp        0      0 0.0.0.0:5353            0.0.0.0:*                           3619/chrome         
udp        0      0 0.0.0.0:5353            0.0.0.0:*                           -                   
udp        0      0 0.0.0.0:26517           0.0.0.0:*                           -                   
udp6       0      0 :::11970                :::*                                -                   
udp6       0      0 :::111                  :::*                                -                   
udp6       0      0 :::123                  :::*                                -                   
udp6       0      0 ::1:323                 :::*                                -                   
udp6       0      0 :::734                  :::*                                -                   
udp6       0      0 :::51108                :::*                                -                   

[onega@localhost flume]$ LD_LIBRARY_PATH=/home/onega/gcc-6.3.0/lib64:$LD_LIBRARY_PATH ./helloflume --loop 20 --batch 50
try_flume successfully finished 20 operations with apache flume via thrift interface
flume took 78.5235 ms, or 78 whole milliseconds
        (9 segments)
        (4 segments)
/usr/local/lib/libthrift-0.10.0.so      (7 segments)
/usr/local/lib/libboost_program_options.so.1.59.0       (7 segments)
/usr/local/lib/libboost_system.so.1.59.0        (7 segments)
/usr/lib64/libdl.so.2   (9 segments)
/home/onega/gcc-6.3.0/lib64/libstdc++.so.6      (7 segments)
/home/onega/gcc-6.3.0/lib64/libgcc_s.so.1       (6 segments)
/usr/lib64/libpthread.so.0      (9 segments)
/usr/lib64/libc.so.6    (10 segments)
/usr/lib64/libssl.so.10 (7 segments)
/usr/lib64/libcrypto.so.10      (7 segments)
/usr/lib64/librt.so.1   (9 segments)
/usr/lib64/libm.so.6    (9 segments)
/lib64/ld-linux-x86-64.so.2     (7 segments)
/usr/lib64/libgssapi_krb5.so.2  (7 segments)
/usr/lib64/libkrb5.so.3 (7 segments)
/usr/lib64/libcom_err.so.2      (8 segments)
/usr/lib64/libk5crypto.so.3     (7 segments)
/usr/lib64/libz.so.1    (7 segments)
/usr/lib64/libkrb5support.so.0  (7 segments)
/usr/lib64/libkeyutils.so.1     (7 segments)
/usr/lib64/libresolv.so.2       (9 segments)
/usr/lib64/libselinux.so.1      (8 segments)
/usr/lib64/libpcre.so.1 (7 segments)
/usr/lib64/liblzma.so.5 (7 segments)
