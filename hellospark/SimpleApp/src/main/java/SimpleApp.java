import org.apache.spark.sql.SparkSession;
import org.apache.spark.sql.Dataset;
import java.lang.invoke.MethodHandles;

public class SimpleApp {
  public static void main(String[] args) {
    if (args.length == 0) {
        System.err.println("Please specifiy an existing text file.");
        System.exit(1);
    }
    String logFile = args[0];
    long startTime = System.nanoTime();    
    SparkSession spark = SparkSession.builder().appName("SimpleApp").getOrCreate();
    Dataset<String> logData = spark.read().textFile(logFile).cache();

    long numAs = logData.filter(s -> s.contains("a")).count();
    long numBs = logData.filter(s -> s.contains("b")).count();
 
    long estimatedTime = System.nanoTime() - startTime;
    System.out.println(MethodHandles.lookup().lookupClass().getName() +
                       " Lines with a: " + numAs + ", lines with b: " + numBs);
    System.out.println("Program spent " + estimatedTime/1000000 + " milliseconds\n");
    spark.stop();
  }
}