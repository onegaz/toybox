#include "THBaseService.h"  
#include <iostream>  
#include <vector>  
#include <transport/TSocket.h>  
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>  
#include <string>  
#include <sstream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <ctime>
#include <unistd.h>
#include <boost/program_options.hpp>
#include <iterator>
#include <utility>
#include <memory>
#include <sys/types.h>
#include <ctime>
#include <zookeeper.h>
#include <zookeeper_log.h>

#define gettid() syscall(SYS_gettid)
std::string hbase_thrift2_server_address="localhost";
std::string zookeeper_server_address="localhost";
unsigned short port=9090;               // hbase.regionserver.thrift.port
unsigned short zookeeper_port = 2222;   // hbase.zookeeper.property.clientPort
std::string table_name="t2";
std::string row = "row2";
int scan = 0;   // flag to enable scan of hbase table

std::string reference = R"(http://hbase.apache.org/0.94/book/zookeeper.html
https://blog.cloudera.com/blog/2013/10/what-are-hbase-znodes/
http://hbase.apache.org/book.html#quickstart)";

void scan_table(apache::hadoop::hbase::thrift2::THBaseServiceClient& client, const std::string& table_name) {
    std::cout << __func__ << " start" << std::endl;
    apache::hadoop::hbase::thrift2::TScan scan;
    int scannerId = client.openScanner(table_name, scan);
    std::vector<apache::hadoop::hbase::thrift2::TResult> result;
    do {
        result.clear();
        client.getScannerRows(result, scannerId, 1);
        for(auto &tr: result) {
            std::cout << tr.row << "\t";
            for(auto &it: tr.columnValues)
                std::cout << it << std::endl;
        }
    } while(result.size());
    client.closeScanner(scannerId);
    std::cout << __func__ << " end" << std::endl;
}
int read_hbase() {
    std::cout << __func__ << " start, connect to hbase thrift2 interface " <<hbase_thrift2_server_address<<":"<<port << std::endl;
    boost::shared_ptr<apache::thrift::transport::TSocket> socket(new apache::thrift::transport::TSocket(hbase_thrift2_server_address, port));  
    boost::shared_ptr<apache::thrift::transport::TTransport> transport(new apache::thrift::transport::TBufferedTransport(socket));  
    boost::shared_ptr<apache::thrift::protocol::TProtocol> protocol(new apache::thrift::protocol::TBinaryProtocol(transport));  
    try { 
        transport->open();  
        apache::hadoop::hbase::thrift2::THBaseServiceClient  client(protocol);  
        if (row.length()) {
            apache::hadoop::hbase::thrift2::TResult tresult;  
            apache::hadoop::hbase::thrift2::TGet get;
            std::vector<apache::hadoop::hbase::thrift2::TColumnValue> cvs;
            get.__set_row(row);  
            bool has_row = client.exists(table_name, get);   
            if (has_row) {
                client.get(tresult, table_name, get);
                for(auto &it: tresult.columnValues)
                    std::cout << it << std::endl;
            }
            else 
                std::cout << row << " is not found" << std::endl;
        }
        if (scan)
            scan_table(client, table_name);
        transport->close();  
     } catch (const apache::thrift::TException &tx) {
        std::cerr << "ERROR(exception): " << tx.what() << std::endl;
     }
    std::cout << __func__ << " end" << std::endl;
    return 0; 
}
  
int write_hbase(){
    std::cout << __func__ << " start, connect to hbase thrift2 interface " <<hbase_thrift2_server_address<<":"<<port << std::endl;
    boost::shared_ptr<apache::thrift::transport::TSocket> socket(new apache::thrift::transport::TSocket(hbase_thrift2_server_address, port));  
    boost::shared_ptr<apache::thrift::transport::TTransport> transport(new apache::thrift::transport::TBufferedTransport(socket));  
    boost::shared_ptr<apache::thrift::protocol::TProtocol> protocol(new apache::thrift::protocol::TBinaryProtocol(transport));  
    try {
        char buf[128];
        transport->open();  
        apache::hadoop::hbase::thrift2::THBaseServiceClient  client(protocol);  
        apache::hadoop::hbase::thrift2::TResult tresult;  
        apache::hadoop::hbase::thrift2::TGet get;
        std::vector<apache::hadoop::hbase::thrift2::TPut> puts; 
        for(int i = 0; i < 2; i++) { 
            for(int j = 0; j < 3; j++) {
                apache::hadoop::hbase::thrift2::TPut put;
                std::vector<apache::hadoop::hbase::thrift2::TColumnValue> cvs;
                //put data
                sprintf(buf, "pid-%d-%d", getpid(), j);
                const std::string thisrow(buf); 
                put.__set_row(thisrow);
                apache::hadoop::hbase::thrift2::TColumnValue tcv;
                tcv.__set_family("cf");
                tcv.__set_qualifier("a");
                sprintf(buf, "value from %s %d", __func__, i*1000+ j);
                tcv.__set_value(buf);
                cvs.insert(cvs.end(), tcv);
                put.__set_columnValues(cvs);
                puts.insert(puts.end(), put);
            }
            client.putMultiple(table_name, puts);
            puts.clear();
        }
        transport->close();  
     } catch (const apache::thrift::TException &tx) {
        std::cerr << "ERROR(exception): " << tx.what() << std::endl;
     }
    std::cout << __func__ << " end" << std::endl;
    return 0;
}

const char* get_self_path() {
  static char selfpath[PATH_MAX];
  if(selfpath[0])
    return selfpath;
  char path[PATH_MAX];
  sprintf(path, "/proc/%d/exe", getpid());
  if (readlink(path, selfpath, PATH_MAX) == -1)
    perror("readlink");
    return selfpath;
}

void watcher_func(zhandle_t* zh, int type, int state,
                  const char* path, void* watcherCtx)
{
}

void free_vector(struct String_vector* strvector) {
    if(!strvector)
        return;
    for(int i = 0; i < strvector->count; i++) {
        free(strvector->data[i]);
    }    
    free(strvector -> data);
}

void list_hbase_tables() {
    std::stringstream ss;
    ss << zookeeper_server_address << ":" << zookeeper_port;
    std::string zookeeper_address = ss.str();
    std::cout << __func__ << " start, connect to zookeeper " << zookeeper_address << std::endl;
    int timeout = 2000;
    int flag=0;
    flag |= ZOO_EPHEMERAL;
    flag |= ZOO_SEQUENCE;
    zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
    zhandle_t* zkhandle = zookeeper_init(zookeeper_address.c_str(),
                                         watcher_func, timeout, 0, (void*)"hellozookeeper", 0);
    if (zkhandle == NULL) {
        std::cout << __func__ << " failed to connect " << zookeeper_address << std::endl;
        exit(EXIT_FAILURE);
    }
    int i;
    struct String_vector paths;
    int ret = zoo_get_children(zkhandle,"/hbase/table", 0,&paths);// note: free mem
    if(ret)
    {
        std::cout << "zoo_get_children error " << ret << std::endl;       
    } else {
        for(i = 0;i < paths.count;i++)
            printf("/hbase/table/%s\n",paths.data[i]);  
        free_vector(&paths);  
    }
    zookeeper_close(zkhandle); 
    std::cout << __func__ << " end" << std::endl;   
}

int main(int argc, char **argv) {  
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message")
    ("port", boost::program_options::value<decltype(port)>( &port)->default_value(port), "thrift2 Port number to connect")
    ( "server", boost::program_options::value<decltype(hbase_thrift2_server_address)>(&hbase_thrift2_server_address)->default_value(hbase_thrift2_server_address),
    "HBase thrift2 server address")
    ( "zookeeper server", boost::program_options::value<decltype(zookeeper_server_address)>(&zookeeper_server_address)->default_value(zookeeper_server_address),
    "zookeeper server address")
    ("zookeeper_port", boost::program_options::value<decltype(zookeeper_port)>( &zookeeper_port)->default_value(zookeeper_port), "zookeeper_port number")
    ( "table", boost::program_options::value<decltype(table_name)>(&table_name)->default_value(table_name),
    "Specify table name to access")
    ( "row", boost::program_options::value<decltype(row)>(&row)->default_value(row),
    "Specify row")
    ( "scan", "Perform scan on table if this option is specified")
    ( "put", "Insert rows to table if this option is specified")
    ( "list", "list tables of hbase registered in zookeeper")
    ;
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);
    if(vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }
    if (vm.count("list"))
        list_hbase_tables();
    if(vm.count("put"))
        write_hbase();
    scan = vm.count("scan");
    read_hbase();
    std::cout << get_self_path() << " pid " << getpid() << " exit. Built with g++ "<< __GNUC__<<"." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ <<std::endl;
    return 0;  
}  