package org.myorg;
        
import java.util.*;
import java.net.*;
import java.io.*;        
import java.util.regex.*;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.input.TextInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.output.TextOutputFormat;
import org.apache.hadoop.util.Tool;
import org.apache.hadoop.util.ToolRunner;
// Map -> Combiner -> Partitioner -> Sort -> Shuffle -> Sort -> Reduce
// https://farm3.static.flickr.com/2275/3529146683_c8247ff6db_o.png 
// https://hadoopi.wordpress.com/2013/05/27/understand-recordreader-inputsplit/
// https://hadoopi.wordpress.com/2013/05/31/custom-recordreader-processing-string-pattern-delimited-records/
// It is better to process multiple lines at once
// http://analyticspro.org/2012/08/01/wordcount-with-custom-record-reader-of-textinputformat/
// https://highlyscalable.wordpress.com/2012/02/01/mapreduce-patterns/ 
public class hellohadoop extends Configured implements Tool  {
        
 public static class Map extends Mapper<LongWritable, Text, Text, IntWritable> {
    private final static IntWritable one = new IntWritable(1);
    private Text word = new Text();
    @Override
    public void map(LongWritable key, Text value, Context context) throws IOException, InterruptedException {
        String line = value.toString();
        StringTokenizer tokenizer = new StringTokenizer(line);
        HashMap<String, Integer>  wordcnt = new HashMap<String, Integer>();
        while (tokenizer.hasMoreTokens()) {
            String oneword = tokenizer.nextToken();
            if (wordcnt.containsKey(oneword)==false)
                wordcnt.put(oneword, 1);
            else {
                wordcnt.put(oneword, wordcnt.get(oneword) + 1);
            }
            
        }
        for(String oneword: wordcnt.keySet()) {
            word.set(oneword);
            context.write(word, new IntWritable(wordcnt.get(oneword)));
        }
    }
 } 
        
 public static class Reduce extends Reducer<Text, IntWritable, Text, IntWritable> {
     @Override
    public void reduce(Text key, Iterable<IntWritable> values, Context context) 
      throws IOException, InterruptedException {
        int sum = 0;
        for (IntWritable val : values) {
            sum += val.get();
        }
        context.write(key, new IntWritable(sum));
    }
 }
 int  oldmainbody(String[] args, Configuration conf) throws Exception {    
    Job job =  Job.getInstance(conf, getClass().getSimpleName());
    job.setOutputKeyClass(Text.class);
    job.setOutputValueClass(IntWritable.class);        
    job.setMapperClass(Map.class);
    job.setReducerClass(Reduce.class);        
    job.setInputFormatClass(TextInputFormat.class);
    job.setOutputFormatClass(TextOutputFormat.class);        
    FileInputFormat.addInputPath(job, new Path(args[0]));
    FileOutputFormat.setOutputPath(job, new Path(args[1]));
    job.setJarByClass(getClass()); 
    return job.waitForCompletion(true) ? 0 : 1;
 }

 public static void listhdfs() throws Exception {
     // $HADOOP_HOME/sbin/start-dfs.sh
     String[] environment_variables = {"HADOOP_HOME", "JAVA_HOME"};
     for( String name: environment_variables ) {
        String env = System.getenv(name);
        if(env == null) {
            System.out.format("Environment variable %s is not found\n", name);
        }
     }
    Configuration conf = new Configuration();
    // conf.addResource(new Path(System.getenv("HADOOP_HOME") +"/etc/hadoop/core-site.xml"));
    // conf.addResource(new Path(System.getenv("HADOOP_HOME") +"/etc/hadoop/hdfs-site.xml"));
    FileSystem fs = FileSystem.get(new URI("hdfs://localhost:9000/"), conf);
    FileStatus[] fileStatus = fs.listStatus(new Path("hdfs://localhost:9000/"));
    for(FileStatus status : fileStatus){
        System.out.println(status.getPath().toString());
    } 
}

static void checkprocess(HashMap<String, String> map, String line, String[] patterns) {
    for(String regex: patterns) {
        Pattern p = Pattern.compile(regex);
        Matcher m = p.matcher(line); 
        if(m.matches()) {
            map.put(m.group(2), m.group(1));
        }
    }
}

static void startdfs() throws Exception {
    String[] command = {"bash", System.getenv("HADOOP_HOME")+"/sbin/start-dfs.sh"};
    ProcessBuilder probuilder = new ProcessBuilder( command );
    Process process = probuilder.start();            
    //Read out dir output
    InputStream is = process.getInputStream();
    InputStreamReader isr = new InputStreamReader(is);
    BufferedReader br = new BufferedReader(isr);
    String line;
    System.out.printf("Output of running %s is:\n",
            Arrays.toString(command));
    while ((line = br.readLine()) != null) {
        System.out.println(line);
    }            
    //Wait to get exit value
    try {
        int exitValue = process.waitFor();
        System.out.println("\n\nExit Value is " + exitValue);
    } catch (InterruptedException e) {
        // TODO Auto-generated catch block
        e.printStackTrace();
    } 
}
static void checkfiles(int pid) throws Exception {
    String[] command = {"lsof", "-p", Integer.toString(pid)};
    ProcessBuilder probuilder = new ProcessBuilder( command );
    Process process = probuilder.start();            
    //Read out dir output
    InputStream is = process.getInputStream();
    InputStreamReader isr = new InputStreamReader(is);
    BufferedReader br = new BufferedReader(isr);
    String line;
    System.out.printf("Output of running %s is:\n",
            Arrays.toString(command));
    while ((line = br.readLine()) != null) {
        System.out.println(line);
    }            
    //Wait to get exit value
    try {
        int exitValue = process.waitFor();
        System.out.println("\n\nExit Value is " + exitValue);
    } catch (InterruptedException e) {
        // TODO Auto-generated catch block
        e.printStackTrace();
    }     
}

static void findjar(Class klass) {
    URL location = klass.getResource('/' + klass.getName().replace('.', '/') + ".class");
    System.out.println(location.toString());
}
    public static void main(String[] args) throws Exception {
// $HADOOP_HOME/sbin/start-dfs.sh        
// 6916 NameNode
// 7112 DataNode
// 7373 SecondaryNameNode
    findjar(FileInputFormat.class); // find necessary jar for VSCode, maybe should use http://www.kirkk.com/main/Main/JarAnalyzer 
        String[] dfs_patterns = {"(\\d+) (NameNode)", "(\\d+) (DataNode)", "(\\d+) (SecondaryNameNode)", "(\\d+) (NodeManager)" };
        HashMap<String, String>  hadoop_processes = new HashMap<String, String>();
        System.out.println(System.getProperty( "java.library.path" ));
        ProcessBuilder pb = new ProcessBuilder("jps");
        Process process = pb.start();
        InputStream is = process.getInputStream();
        InputStreamReader isr = new InputStreamReader(is);
        BufferedReader br = new BufferedReader(isr);
        String line;
        System.out.format("Output of running %s is:\n", pb.command().toString());
        while ((line = br.readLine()) != null) {
            System.out.println(line);
            checkprocess(hadoop_processes, line, dfs_patterns);
        }
        for(String key: hadoop_processes.keySet())
            System.out.println(key + " - " + hadoop_processes.get(key));
        if(!hadoop_processes.containsKey("DataNode")) {
            System.out.println("hdfs is not started. please start $HADOOP_HOME/sbin/start-dfs.sh");
            startdfs();
        }            
        else
            System.out.println("hdfs is started. You can stop it by $HADOOP_HOME/sbin/stop-dfs.sh");
// $HADOOP_HOME/sbin/start-yarn.sh
// 7883 NodeManager
// bin/hbase-1.3.0/bin/hbase thrift2 -threadpool start
// 19029 ThriftServer
// bin/hbase-1.3.0/bin/start-hbase.sh
// 19401 HMaster
// [onega@localhost kafka_2.11-0.10.1.1]$ bin/zookeeper-server-start.sh config/zookeeper.properties
// 3961 QuorumPeerMain
// [onega@localhost kafka_2.11-0.10.1.1]$ bin/kafka-server-start.sh config/server.properties
// 4286 Kafka

        if(args.length<3) {
            listhdfs();
            java.lang.management.RuntimeMXBean runtime = 
            java.lang.management.ManagementFactory.getRuntimeMXBean();
            java.lang.reflect.Field jvm = runtime.getClass().getDeclaredField("jvm");
            jvm.setAccessible(true);
            sun.management.VMManagement mgmt = (sun.management.VMManagement) jvm.get(runtime);
            java.lang.reflect.Method pid_method =  
                mgmt.getClass().getDeclaredMethod("getProcessId");
            pid_method.setAccessible(true);
            int pid = (Integer) pid_method.invoke(mgmt);
            System.out.printf("pid %d classpath %s\n", pid, System.getProperty( "java.class.path" ));
            checkfiles(pid);
            System.exit(1);
        }
        int res = ToolRunner.run(new Configuration(), new hellohadoop(), args);
        System.exit(res);
    }
 
    @Override
    public int run(String[] args) throws Exception { 
        // When implementing tool
        Configuration conf = this.getConf(); 
        return oldmainbody(args, conf);
    }        
}

// HADOOP_CLASSPATH=$JAVA_HOME/lib/tools.jar /home/onega/bin/hadoop-2.7.3/bin/hadoop com.sun.tools.javac.Main *.java -d $PWD
// https://hadoop.apache.org/docs/stable/hadoop-mapreduce-client/hadoop-mapreduce-client-core/MapReduceTutorial.html
// javac -cp $(/home/onega/bin/hadoop-2.7.3/bin/hadoop classpath) hellohadoop.java -Xlint:deprecation -d $PWD
// [onega@localhost java]$ jar cvf hellohadoop.jar org/myorg/*
// added manifest
// adding: org/myorg/hellohadoop.class(in = 1525) (out= 745)(deflated 51%)
// adding: org/myorg/hellohadoop$Map.class(in = 1881) (out= 786)(deflated 58%)
// adding: org/myorg/hellohadoop$Reduce.class(in = 1653) (out= 690)(deflated 58%)
// [onega@localhost java]$ java -cp ./hellohadoop.jar:$(/home/onega/bin/hadoop-2.7.3/bin/hadoop classpath) org.myorg.hellohadoop
// :/usr/local/lib:/usr/local/lib64:/usr/lib64:/usr/local/lib:/usr/local/lib64:/usr/lib64:/usr/java/packages/lib/amd64:/usr/lib64:/lib64:/lib:/usr/lib
// [onega@localhost java]$ 
