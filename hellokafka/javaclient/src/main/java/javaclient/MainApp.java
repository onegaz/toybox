package javaclient;

import java.util.ArrayList;
import java.util.Properties;
import java.util.Set;
import java.util.Collection;
import java.util.Arrays;
import java.lang.invoke.MethodHandles;
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
	        producer.send(new ProducerRecord<String, String>(topic, 
	        		"key " + Integer.toString(i), "value " + methodName + " " + Long.toString(System.nanoTime())));
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
    	topicName = MethodHandles.lookup().lookupClass().getName();
        Properties prop = new Properties();
        prop.put("bootstrap.servers", "localhost:9092");
        AdminClient admin = AdminClient.create(prop);
        createTopic(admin, topicName);
        ListTopicsResult topics = admin.listTopics();
        Set<String> topicNames = topics.names().get();
        for(String topic: topicNames) {
            System.out.println(topic);
        }
        System.out.println("after listTopics");
        produceMessages(topicName, 8);
        consume(topicName);
        System.exit(0);
    }
}