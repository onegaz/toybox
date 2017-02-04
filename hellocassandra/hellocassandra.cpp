#include "Cassandra.h"
#include "transport/TSocket.h"
#include "transport/TBufferTransports.h"
#include "protocol/TBinaryProtocol.h"
#include <iostream>
#include <sstream>

using namespace apache::thrift::transport;
using namespace org::apache::cassandra;

void batch_insert_cassandra(CassandraClient& cass) {
    struct timeval tv;
    gettimeofday(&tv, NULL);  // #include <sys/time.h>
    auto get_insert_cql = [](int seconds, int tv_usec) {
      std::stringstream ss;
      ss << "insert into user_profiles (user_id, first_name, last_name, year_of_birth) ";
      ss << "values( "; // values('user1', 'first1', 'last1', 2017);
      ss << "'user" << seconds <<"-"<<tv_usec <<"', 'first"<< tv_usec << "', 'last" << seconds 
      <<"-" << tv_usec << "', "
      << 2017
      << ")";
      return ss.str();
    };
    std::stringstream ss;
    ss << "BEGIN BATCH" << std::endl;
    for(int i=0; i<5; i++)
      ss << get_insert_cql(tv.tv_sec, tv.tv_usec+i) << ";" << std::endl;
    ss << "APPLY BATCH;" << std::endl;
    std::cout << ss.str() << std::endl;    
    CqlResult _return;
    cass.execute_cql3_query( _return, ss.str(), Compression::NONE, ConsistencyLevel::ONE);
    std::cout << "num " << _return.num << ", rows " << _return.rows.size() << std::endl;
    std::cout << __func__ << " end"    << std::endl;
}

int main(int argc, char *argv[]){
  std::cout << argv[0] << " pid " << getpid()<< " start" << std::endl;
  // todo yaml-cpp/yaml.h 
  try{
      unsigned short rpc_port = 9160; // see rpc_port: 9160 in apache-cassandra-3.9/conf/cassandra.yaml
// ./conf/cassandra.yaml:637:# Whether to start the thrift rpc server.
// ./conf/cassandra.yaml-638-start_rpc: false
// Start Cassandra in the foreground by invoking bin/cassandra -f from the command line. 
// Press “Control-C” to stop Cassandra. Start Cassandra in the background by invoking bin/cassandra 
// from the command line. Invoke kill pid or pkill -f CassandraDaemon to stop Cassandra, 
// where pid is the Cassandra process id, which you can find for example by invoking pgrep -f CassandraDaemon.
// Verify that Cassandra is running by invoking bin/nodetool status from the command line.
// Configuration files are located in the conf sub-directory.
// Since Cassandra 2.1, log and data directories are located in the logs and data sub-directories respectively. 
// Older versions defaulted to /var/log/cassandra and /var/lib/cassandra. Due to this, it is necessary to 
// either start Cassandra with root privileges or change conf/cassandra.yaml to use directories owned by the 
// current user, as explained below in the section on changing the location of directories.

    boost::shared_ptr<TTransport> socket = boost::shared_ptr<TSocket>(new TSocket("127.0.0.1", rpc_port));
    boost::shared_ptr<TTransport> tr = boost::shared_ptr<TFramedTransport>(new TFramedTransport (socket));
    boost::shared_ptr<apache::thrift::protocol::TProtocol> p = boost::shared_ptr<apache::thrift::protocol::TBinaryProtocol>(new apache::thrift::protocol::TBinaryProtocol(tr));
    CassandraClient cass(p);
    tr->open();
    auto execute_cql_select = [](CassandraClient& cass, const std::string& query) {
      CqlResult _return;
      // std::string query="select * from hellocassdra_keyspace.user_profiles";
      cass.execute_cql3_query( _return, query, Compression::NONE, ConsistencyLevel::ONE);
      std::cout << "num " << _return.num << ", rows " << _return.rows.size() << std::endl;
      // _return.printTo(std::cout);
      for(auto row: _return.rows) {
        for( auto column: row.columns) {
          std::cout << column.value << "\t";
        }
        std::cout << std::endl;
      }
      // std::cout << _return << std::endl;   
      std::cout << std::endl;   
    };
    // SELECT * FROM system_schema.keyspaces;
    execute_cql_select(cass, "SELECT * FROM system_schema.keyspaces");
    // SELECT keyspace_name, table_name FROM system_schema.tables LIMIT 2; 
    execute_cql_select(cass, "SELECT keyspace_name, table_name FROM system_schema.tables LIMIT 2");
    cass.set_keyspace("hellocassdra_keyspace");
    batch_insert_cassandra(cass);

    std::string query="select * from hellocassdra_keyspace.user_profiles";
    execute_cql_select(cass, query);

    tr->close();
  }catch(const TTransportException& te){
    printf("TTransportException: %s  [%d]\n", te.what(), te.getType());
  }catch(const InvalidRequestException& ire){
    printf("InvalidRequestException: %s  [%s]\n", ire.what(), ire.why.c_str());
  }catch(const NotFoundException& nfe){
    printf("NotFoundException: %s\n", nfe.what());
  }
  std::cout << argv[0] << " exit" << std::endl;
  return 0;
}

// it works with /usr/local/lib/libthrift.so.0.9.3 or /usr/local/lib/libthrift-0.10.0.so