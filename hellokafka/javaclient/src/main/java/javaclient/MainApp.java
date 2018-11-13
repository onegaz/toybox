package javaclient;

import java.util.ArrayList;
import java.util.Properties;
import java.util.Set;
import java.util.Collection;
import java.util.List;
import java.util.Arrays;
import java.lang.invoke.MethodHandles;
import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Structure;

import org.apache.kafka.clients.admin.AdminClient;
import org.apache.kafka.clients.admin.CreateTopicsResult;
import org.apache.kafka.clients.admin.ListTopicsResult;
import org.apache.kafka.clients.admin.NewTopic;
import org.apache.kafka.clients.consumer.ConsumerRebalanceListener;
import org.apache.kafka.clients.consumer.ConsumerRecord;
import org.apache.kafka.clients.consumer.ConsumerRecords;
import org.apache.kafka.clients.consumer.KafkaConsumer;
import org.apache.kafka.clients.producer.KafkaProducer;
import org.apache.kafka.clients.producer.Producer;
import org.apache.kafka.clients.producer.ProducerRecord;
import org.apache.kafka.common.TopicPartition;

// ~/bin/kafka_2.12-1.1.0$ bin/zookeeper-server-start.sh config/zookeeper.properties
// jps report QuorumPeerMain
// ~/bin/kafka_2.12-1.1.0$ bin/kafka-server-start.sh config/server.properties
// jps report Kafka
// /usr/lib/jvm/java-8-openjdk-amd64/bin/java -jar ~/oss/toybox/hellokafka/javaclient/target/kafka-javaclient-1.0.jar
/*
~/bin/kafka_2.12-1.1.0$ bin/kafka-delete-records.sh --bootstrap-server localhost:9092 --offset-json-file ~/oss/toybox/hellokafka/javaclient/offset.json
Executing records delete operation
Records delete operation completed:
partition: javaclient.MainApp-0	low_watermark: 1
*/
// ~/bin/kafka_2.12-1.1.0$ bin/kafka-server-stop.sh
// ~/bin/kafka_2.12-1.1.0$ bin/zookeeper-server-stop.sh

public class MainApp {
    public interface CLibrary extends Library {
        CLibrary INSTANCE = (CLibrary)Native.loadLibrary("c", CLibrary.class);

        int syscall(int number);
        int getpid ();
        int getrusage(int who, rusage usage);
    }
    public static class timeval extends Structure {
        public long tv_sec;
        public int tv_usec;

        @Override
        protected List getFieldOrder() {
            return Arrays.asList("tv_sec", "tv_usec");
        }

        @Override
        public String toString() {
            return String.format("%d.%06d", tv_sec, tv_usec);
        }
    } 
    public static class rusage extends Structure {
        public timeval ru_utime;	/* user CPU time used */
        public timeval ru_stime;	/* system CPU time used */
        public long ru_maxrss;		/* maximum resident set size */
        public long ru_ixrss;		/* integral shared memory size */
        public long ru_idrss;		/* integral unshared data size */
        public long ru_isrss;		/* integral unshared stack size */
        public long ru_minflt;		/* page reclaims (soft page faults) */
        public long ru_majflt;		/* page faults (hard page faults) */
        public long ru_nswap;		/* swaps */
        public long ru_inblock;		/* block input operations */
        public long ru_oublock;		/* block output operations */
        public long ru_msgsnd;		/* IPC messages sent */
        public long ru_msgrcv;		/* IPC messages received */
        public long ru_nsignals;	/* signals received */
        public long ru_nvcsw;		/* voluntary context switches */
        public long ru_nivcsw;		/* involuntary context switches */

        @Override
        protected List getFieldOrder() {
            return Arrays.asList(
                "ru_utime",
                "ru_stime",
                "ru_maxrss",
                "ru_ixrss",
                "ru_idrss",
                "ru_isrss",
                "ru_minflt",
                "ru_majflt",
                "ru_nswap",
                "ru_inblock",
                "ru_oublock",
                "ru_msgsnd",
                "ru_msgrcv",
                "ru_nsignals",
                "ru_nvcsw",
                "ru_nivcsw");
        }
    }    
    // wrapper function of gettid()
    private static int gettid() {
        final int SYS_gettid = 186;
        return CLibrary.INSTANCE.syscall(SYS_gettid);
    }
    // wrapper function of getrusage()
    private static rusage getrusage() {
    	final int RUSAGE_SELF = 0; // https://elixir.bootlin.com/linux/latest/source/include/uapi/linux/resource.h#L19
        final int RUSAGE_THREAD = 1;
        rusage usage = new rusage();
        CLibrary.INSTANCE.getrusage(RUSAGE_SELF, usage);
        return usage;
    }
    
	static void createTopic(AdminClient admin, String topic) throws Exception
	{
	    ArrayList<NewTopic> topics = new ArrayList<NewTopic>();
	    NewTopic newTopic = new NewTopic(topic, 1, (short) 1);
	    topics.add(newTopic);
	    CreateTopicsResult result = admin.createTopics(topics);
	    String methodName = new Object() {}
	      .getClass()
	      .getEnclosingMethod()
	      .getName();
	    System.out.println(methodName + " done");
	}
	
	static int getpid() throws Exception
	{
		java.lang.management.RuntimeMXBean runtime = 
			    java.lang.management.ManagementFactory.getRuntimeMXBean();
		java.lang.reflect.Field jvm = runtime.getClass().getDeclaredField("jvm");
		jvm.setAccessible(true);
		sun.management.VMManagement mgmt =  
		    (sun.management.VMManagement) jvm.get(runtime);
		java.lang.reflect.Method pid_method =  
		    mgmt.getClass().getDeclaredMethod("getProcessId");
		pid_method.setAccessible(true);

		int pid = (Integer) pid_method.invoke(mgmt);
		return pid;
	}
	
	static void produceMessages(String topic, int count) throws Exception
	{
	    Properties props = new Properties();
	    props.put("bootstrap.servers", "localhost:9092");
	    props.put("acks", "all");
	    props.put("retries", 0);
	    props.put("batch.size", 16384);
	    props.put("linger.ms", 1);
	    props.put("buffer.memory", 33554432);
	    props.put("key.serializer", "org.apache.kafka.common.serialization.StringSerializer");
	    props.put("value.serializer", "org.apache.kafka.common.serialization.StringSerializer");
	    
	    String methodName = new Object() {}
	      .getClass()
	      .getEnclosingMethod()
	      .getName();
	      
	    Producer<String, String> producer = new KafkaProducer<String, String>(props);
	    for (int i = 0; i < count; i++)
	    {
	    	rusage usage = getrusage();
	    	StringBuilder sb = new StringBuilder();
	    	sb.append(System.currentTimeMillis());
	    	sb.append(",");
	    	sb.append(usage.ru_nvcsw);
	    	sb.append(",");
	    	sb.append(usage.ru_nivcsw);	    	
//	    	sb.append(",");
//	    	sb.append(usage.ru_isrss);
	    	
	        producer.send(new ProducerRecord<String, String>(topic, 
	        		"key " + Long.toString(System.currentTimeMillis()), 
	        		sb.toString()), new org.apache.kafka.clients.producer.Callback() {
                @Override
                public void onCompletion(org.apache.kafka.clients.producer.RecordMetadata metadata, Exception exception) {
                    if (exception != null) {
                        System.out.println("Failed to send message - exception " + exception);
                    }
                }
            });
	        Thread.sleep(200);
	    }
	    producer.close();
	    System.out.println("KafkaProducer done");
	}
	
	static void consume(String topic)
	{
		Properties props = new Properties();
	    props.put("bootstrap.servers", "localhost:9092");
	    props.put("group.id", "test");
	    props.put("enable.auto.commit", "true");
	    props.put("auto.commit.interval.ms", "200");
	    props.put("key.deserializer", "org.apache.kafka.common.serialization.StringDeserializer");
	    props.put("value.deserializer", "org.apache.kafka.common.serialization.StringDeserializer");
	    final KafkaConsumer<String, String> consumer = new KafkaConsumer<String,String>(props);
	    consumer.subscribe(Arrays.asList(topic),new ConsumerRebalanceListener() {
	        public void onPartitionsRevoked(Collection<TopicPartition> collection) {
				String methodName = new Object() {
				}.getClass().getEnclosingMethod().getName();
				System.out.println(methodName);
	        }
	        public void onPartitionsAssigned(Collection<TopicPartition> collection) {
				String methodName = new Object() {
				}.getClass().getEnclosingMethod().getName();
				System.out.println(methodName);
	            consumer.seekToBeginning(collection);
	        }
	    });
	    int tryCount = 0;
	    int maxFailCount = 90;
	    while (tryCount<maxFailCount) {
	        ConsumerRecords<String, String> records = consumer.poll(100);
	        for (ConsumerRecord<String, String> record : records)
	            System.out.printf("offset = %d, key = %s, value = %s%n", record.offset(), record.key(), record.value());
	        if (records.count()==0)
	        	tryCount++;
	        else
	        	tryCount = 0;
	    }
	    System.out.println("KafkaConsumer done");
	}
	static String topicName;
    public static void main(String[] args) throws Exception {
    	System.out.println("KafkaConsumer pid " + new Integer(CLibrary.INSTANCE.getpid()).toString());
    	
    	topicName = MethodHandles.lookup().lookupClass().getName();
        Properties prop = new Properties();
        prop.put("bootstrap.servers", "localhost:9092");
        AdminClient admin = AdminClient.create(prop);
        
        ListTopicsResult topics = admin.listTopics();
        Set<String> topicNames = topics.names().get();
        for(String topic: topicNames) {
            System.out.println(topic);
        }
        System.out.println("after listTopics");
        if (!topicNames.contains(topicName))
        {
        	createTopic(admin, topicName);
        }
        produceMessages(topicName, 60);
//        consume(topicName);
        System.out.println("bin/kafka-topics.sh --list --zookeeper localhost:2181");
        System.exit(0);
    }
}