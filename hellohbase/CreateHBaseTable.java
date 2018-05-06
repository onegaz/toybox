import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.hbase.HBaseConfiguration;
import org.apache.hadoop.hbase.HColumnDescriptor;
import org.apache.hadoop.hbase.HTableDescriptor;
import org.apache.hadoop.hbase.client.HBaseAdmin;
import org.apache.hadoop.hbase.client.Admin;
import org.apache.hadoop.hbase.client.Connection;
import org.apache.hadoop.hbase.client.ConnectionFactory;
import org.apache.hadoop.hbase.TableName;

public class CreateHBaseTable {
	public static void main(String[] args) throws java.io.IOException {
		if (args.length < 2) {
			System.out.println(
				"Usage: java -cp `~/bin/hbase-1.3.0/bin/hbase classpath`:. CreateHBaseTable <TableName> <ColumnName>");
			return;
		}
//		Configuration hconfig = HBaseConfiguration.create(new Configuration());
//		hconfig.addResource("/home/onega/bin/hbase-1.3.0/conf/hbase-site.xml");
//		hconfig.set("hbase.master", "localhost:60000");
		Configuration hconfig = HBaseConfiguration.create();
		Connection connection = ConnectionFactory.createConnection(hconfig);
		Admin hbase_admin = connection.getAdmin();
		HTableDescriptor htable = new HTableDescriptor(TableName.valueOf(args[0]));
		for (int i = 1; i < args.length; i++)
			htable.addFamily(new HColumnDescriptor(args[i]));
		System.out.format("Creating Table %s...", args[0]);
		hbase_admin.createTable(htable);
		System.out.println("Done!");
	}
}
// javac -cp `~/bin/hbase-1.3.0/bin/hbase classpath` -Xlint:deprecation CreateHBaseTable.java
