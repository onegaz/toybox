#include <string>
#include "Cassandra.h"
#include "transport/TSocket.h"
#include "transport/TBufferTransports.h"
#include "protocol/TBinaryProtocol.h"
#include <iostream>

using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using namespace org::apache::cassandra;

int main(int argc, char *argv[]){
  std::cout << argv[0] << " pid " << getpid()<< " start" << std::endl;
  // todo yaml-cpp/yaml.h 
  // find all keyspace/table from cassandra
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
    boost::shared_ptr<TProtocol> p = boost::shared_ptr<TBinaryProtocol>(new TBinaryProtocol(tr));
    CassandraClient cass(p);
    tr->open();

//    cass.set_keyspace("hellocassdra_keyspace");
    CqlResult _return;
    std::string query="select * from hellocassdra_keyspace.user_profiles";
    cass.execute_cql3_query( _return, query, Compression::NONE, ConsistencyLevel::ONE);
    std::cout << "num " << _return.num << std::endl;
    _return.printTo(std::cout);
    std::cout << std::endl;

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