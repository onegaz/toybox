#!/bin/bash

#PATH=/usr/lib/jvm/java-9-oracle/bin:$PATH
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
export PATH=/usr/lib/jvm/java-8-openjdk-amd64/bin:$PATH

java -version

~/bin/apache-maven-3.5.3/bin/mvn clean package

~/bin/spark-2.2.1-bin-hadoop2.7/bin/spark-submit --class SimpleApp target/simple-spark-1.0.jar ~/bin/apache-maven-3.5.3/README.txt

exit 0

https://stackoverflow.com/questions/36024565/how-do-i-pass-program-argument-to-main-function-in-running-spark-submit-with-a-j?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa

# java9 encounter exception: 
Exception in thread "main" java.lang.IllegalArgumentException
	at org.apache.xbean.asm5.ClassReader.<init>(Unknown Source)
	at org.apache.xbean.asm5.ClassReader.<init>(Unknown Source)
	at org.apache.xbean.asm5.ClassReader.<init>(Unknown Source)
	at org.apache.spark.util.ClosureCleaner$.getClassReader(ClosureCleaner.scala:46)
	at org.apache.spark.util.FieldAccessFinder$$anon$3$$anonfun$visitMethodInsn$2.apply(ClosureCleaner.scala:443)
	at org.apache.spark.util.FieldAccessFinder$$anon$3$$anonfun$visitMethodInsn$2.apply(ClosureCleaner.scala:426)
	at scala.collection.TraversableLike$WithFilter$$anonfun$foreach$1.apply(TraversableLike.scala:733)
	at scala.collection.mutable.HashMap$$anon$1$$anonfun$foreach$2.apply(HashMap.scala:103)
	at scala.collection.mutable.HashMap$$anon$1$$anonfun$foreach$2.apply(HashMap.scala:103)
	at scala.collection.mutable.HashTable$class.foreachEntry(HashTable.scala:230)
	at scala.collection.mutable.HashMap.foreachEntry(HashMap.scala:40)
	at scala.collection.mutable.HashMap$$anon$1.foreach(HashMap.scala:103)
	at scala.collection.TraversableLike$WithFilter.foreach(TraversableLike.scala:732)
	at org.apache.spark.util.FieldAccessFinder$$anon$3.visitMethodInsn(ClosureCleaner.scala:426)
	at org.apache.xbean.asm5.ClassReader.a(Unknown Source)
	at org.apache.xbean.asm5.ClassReader.b(Unknown Source)
	at org.apache.xbean.asm5.ClassReader.accept(Unknown Source)
	at org.apache.xbean.asm5.ClassReader.accept(Unknown Source)
	at org.apache.spark.util.ClosureCleaner$$anonfun$org$apache$spark$util$ClosureCleaner$$clean$14.apply(ClosureCleaner.scala:257)
	at org.apache.spark.util.ClosureCleaner$$anonfun$org$apache$spark$util$ClosureCleaner$$clean$14.apply(ClosureCleaner.scala:256)
	at scala.collection.immutable.List.foreach(List.scala:381)
	at org.apache.spark.util.ClosureCleaner$.org$apache$spark$util$ClosureCleaner$$clean(ClosureCleaner.scala:256)
	at org.apache.spark.util.ClosureCleaner$.clean(ClosureCleaner.scala:156)
	at org.apache.spark.SparkContext.clean(SparkContext.scala:2294)
	at org.apache.spark.SparkContext.runJob(SparkContext.scala:2068)
	at org.apache.spark.SparkContext.runJob(SparkContext.scala:2094)
	at org.apache.spark.rdd.RDD$$anonfun$collect$1.apply(RDD.scala:936)
	at org.apache.spark.rdd.RDDOperationScope$.withScope(RDDOperationScope.scala:151)
	at org.apache.spark.rdd.RDDOperationScope$.withScope(RDDOperationScope.scala:112)
	at org.apache.spark.rdd.RDD.withScope(RDD.scala:362)
	at org.apache.spark.rdd.RDD.collect(RDD.scala:935)
	at org.apache.spark.sql.execution.SparkPlan.executeCollect(SparkPlan.scala:278)
	at org.apache.spark.sql.Dataset$$anonfun$count$1.apply(Dataset.scala:2435)
	at org.apache.spark.sql.Dataset$$anonfun$count$1.apply(Dataset.scala:2434)
	at org.apache.spark.sql.Dataset$$anonfun$55.apply(Dataset.scala:2842)
	at org.apache.spark.sql.execution.SQLExecution$.withNewExecutionId(SQLExecution.scala:65)
	at org.apache.spark.sql.Dataset.withAction(Dataset.scala:2841)
	at org.apache.spark.sql.Dataset.count(Dataset.scala:2434)
	at SimpleApp.main(SimpleApp.java:10)
	at java.base/jdk.internal.reflect.NativeMethodAccessorImpl.invoke0(Native Method)
	at java.base/jdk.internal.reflect.NativeMethodAccessorImpl.invoke(NativeMethodAccessorImpl.java:62)
	at java.base/jdk.internal.reflect.DelegatingMethodAccessorImpl.invoke(DelegatingMethodAccessorImpl.java:43)
	at java.base/java.lang.reflect.Method.invoke(Method.java:564)
	at org.apache.spark.deploy.SparkSubmit$.org$apache$spark$deploy$SparkSubmit$$runMain(SparkSubmit.scala:775)
	at org.apache.spark.deploy.SparkSubmit$.doRunMain$1(SparkSubmit.scala:180)
	at org.apache.spark.deploy.SparkSubmit$.submit(SparkSubmit.scala:205)
	at org.apache.spark.deploy.SparkSubmit$.main(SparkSubmit.scala:119)
	at org.apache.spark.deploy.SparkSubmit.main(SparkSubmit.scala)
