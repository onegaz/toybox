#include "THBaseService.h"  
#include <iostream>  
#include <vector>  
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <sstream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <jni.h>
#include <ctime>
#include <unistd.h>
#include <boost/program_options.hpp>
#include <boost/core/demangle.hpp>
#include <iterator>
#include <utility>
#include <memory>
#include <chrono>
#include <unordered_map>
#include <type_traits>
#include <sys/types.h>
#include <ctime>
#include <zookeeper.h>
#include <zookeeper_log.h>

#define gettid() syscall(SYS_gettid)
std::string hbase_thrift2_server_address="localhost";
std::string zookeeper_server_address="localhost";
unsigned short port=9090;               // hbase.regionserver.thrift.port
unsigned short zookeeper_port = 2222;   // hbase.zookeeper.property.clientPort
std::string table_name="jni_t1";
std::string column_family = "jni_cf";
std::string row = "row2";
int scan = 0;   // flag to enable scan of hbase table

std::string reference = R"(http://hbase.apache.org/0.94/book/zookeeper.html
https://blog.cloudera.com/blog/2013/10/what-are-hbase-znodes/
http://hbase.apache.org/book.html#quickstart
in hbase-site.xml:
set hbase.regionserver.thrift.port 
set hbase.zookeeper.property.clientPort

~/bin/zookeeper-3.4.12/src/c$ ./configure --enable-debug
~/bin/zookeeper-3.4.12/src/c$ make

bin/hbase-2.0.0/bin/start-hbase.sh
bin/hbase-2.0.0/bin/hbase thrift2 -threadpool start

LD_LIBRARY_PATH=~/oss/thrift-0.11.0/lib/cpp/.libs:~/bin/zookeeper-3.4.12/src/c/.libs ./hellohbase --put --scan
write_hbase start, connect to hbase thrift2 interface localhost:9090
write_hbase end
it took 56 milliseconds to call write_hbase
read_hbase start, connect to hbase thrift2 interface localhost:9090
row2 is not found
scan_table start
pid-18283-0	TColumnValue(family=cf, qualifier=a, value=value from write_hbase 1000, timestamp=1525320957269, tags=<null>)
pid-18283-1	TColumnValue(family=cf, qualifier=a, value=value from write_hbase 1001, timestamp=1525320957269, tags=<null>)
pid-18283-2	TColumnValue(family=cf, qualifier=a, value=value from write_hbase 1002, timestamp=1525320957269, tags=<null>)
scan_table end
read_hbase end
it took 29 milliseconds to call read_hbase
/home/onzhang/oss/toybox/hellohbase/hellohbase pid 18283 exit. Built with g++ 5.4.0
JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64 HBASE_HOME=${HOME}/bin/hbase-2.0.0 LD_LIBRARY_PATH=${JAVA_HOME}/jre/lib/amd64/server:~/oss/thrift-0.11.0/lib/cpp/.libs:~/bin/zookeeper-3.4.12/src/c/.libs ./hellohbase --put --scan
)";

void scan_table(apache::hadoop::hbase::thrift2::THBaseServiceClient& client,
                const std::string& table_name)
{
    std::cout << __func__ << " start" << std::endl;
    apache::hadoop::hbase::thrift2::TScan scan;
    int scannerId = client.openScanner(table_name, scan);
    std::vector<apache::hadoop::hbase::thrift2::TResult> result;
    do
    {
        result.clear();
        client.getScannerRows(result, scannerId, 1);
        for (auto& tr : result)
        {
            std::cout << tr.row << "\t";
            for (auto& it : tr.columnValues)
                std::cout << it << std::endl;
        }
    } while (result.size());
    client.closeScanner(scannerId);
    std::cout << __func__ << " end" << std::endl;
}

int read_hbase()
{
    std::cout << __func__ << " start, connect to hbase thrift2 interface "
              << hbase_thrift2_server_address << ":" << port << std::endl;
    std::shared_ptr<apache::thrift::transport::TSocket> socket(
        new apache::thrift::transport::TSocket(hbase_thrift2_server_address, port));
    std::shared_ptr<apache::thrift::transport::TTransport> transport(
        new apache::thrift::transport::TBufferedTransport(socket));
    std::shared_ptr<apache::thrift::protocol::TProtocol> protocol(
        new apache::thrift::protocol::TBinaryProtocol(transport));
    transport->open();
    apache::hadoop::hbase::thrift2::THBaseServiceClient client(protocol);
    if (row.length())
    {
        apache::hadoop::hbase::thrift2::TResult tresult;
        apache::hadoop::hbase::thrift2::TGet get;
        std::vector<apache::hadoop::hbase::thrift2::TColumnValue> cvs;
        get.__set_row(row);
        bool has_row = client.exists(table_name, get);
        if (has_row)
        {
            client.get(tresult, table_name, get);
            for (auto& it : tresult.columnValues)
                std::cout << it << std::endl;
        }
        else
            std::cout << row << " is not found" << std::endl;
    }
    if (scan)
        scan_table(client, table_name);
    transport->close();
    std::cout << __func__ << " end" << std::endl;
    return 0;
}

int write_hbase()
{
    std::cout << __func__ << " start, connect to hbase thrift2 interface "
              << hbase_thrift2_server_address << ":" << port << std::endl;
    std::shared_ptr<apache::thrift::transport::TSocket> socket(
        new apache::thrift::transport::TSocket(hbase_thrift2_server_address, port));
    std::shared_ptr<apache::thrift::transport::TTransport> transport(
        new apache::thrift::transport::TBufferedTransport(socket));
    std::shared_ptr<apache::thrift::protocol::TProtocol> protocol(
        new apache::thrift::protocol::TBinaryProtocol(transport));
    char buf[128];
    transport->open();
    apache::hadoop::hbase::thrift2::THBaseServiceClient client(protocol);
    apache::hadoop::hbase::thrift2::TResult tresult;
    apache::hadoop::hbase::thrift2::TGet get;
    std::vector<apache::hadoop::hbase::thrift2::TPut> puts;
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            apache::hadoop::hbase::thrift2::TPut put;
            std::vector<apache::hadoop::hbase::thrift2::TColumnValue> cvs;
            // put data
            sprintf(buf, "pid-%d-%d", getpid(), j);
            const std::string thisrow(buf);
            put.__set_row(thisrow);
            apache::hadoop::hbase::thrift2::TColumnValue tcv;
            tcv.__set_family("cf");
            tcv.__set_qualifier("a");
            sprintf(buf, "value from %s %d", __func__, i * 1000 + j);
            tcv.__set_value(buf);
            cvs.insert(cvs.end(), tcv);
            put.__set_columnValues(cvs);
            puts.insert(puts.end(), put);
        }
        client.putMultiple(table_name, puts);
        puts.clear();
    }
    transport->close();
    std::cout << __func__ << " end" << std::endl;
    return 0;
}

const char* get_self_path()
{
    static char selfpath[PATH_MAX];
    if (selfpath[0])
        return selfpath;
    char path[PATH_MAX];
    sprintf(path, "/proc/%d/exe", getpid());
    if (readlink(path, selfpath, PATH_MAX) == -1)
        perror("readlink");
    return selfpath;
}

void watcher_func(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx)
{
}

void free_vector(struct String_vector* strvector)
{
    if (!strvector)
        return;
    for (int i = 0; i < strvector->count; i++)
    {
        free(strvector->data[i]);
    }
    free(strvector->data);
}

void list_hbase_tables()
{
    std::stringstream ss;
    ss << zookeeper_server_address << ":" << zookeeper_port;
    std::string zookeeper_address = ss.str();
    std::cout << __func__ << " start, connect to zookeeper " << zookeeper_address
              << std::endl;
    int timeout = 2000;
    int flag = 0;
    flag |= ZOO_EPHEMERAL;
    flag |= ZOO_SEQUENCE;
    zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
    zhandle_t* zkhandle = zookeeper_init(zookeeper_address.c_str(), watcher_func, timeout,
                                         0, (void*) "hellozookeeper", 0);
    if (zkhandle == NULL)
    {
        std::cout << __func__ << " failed to connect " << zookeeper_address << std::endl;
        exit(EXIT_FAILURE);
    }

    struct String_vector paths;
    int ret = zoo_get_children(zkhandle, "/hbase/table", 0, &paths); // note: free mem
    if (ret)
    {
        std::cout << "zoo_get_children error " << ret << std::endl;
    }
    else
    {
        for (int i = 0; i < paths.count; i++)
            printf("/hbase/table/%s\n", paths.data[i]);
        free_vector(&paths);
    }
    zookeeper_close(zkhandle);
    std::cout << __func__ << " end" << std::endl;
}

jstring NewJString(JNIEnv* env, const char* str)
{
	if(!env || !str)
		return 0;

	jstring js = env->NewStringUTF(str);

	return js;
}

std::string exec(const std::string& cmd)
{
    std::array<char, 32*1024> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
        throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get()))
    {
        if (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

class java_method
{
public:
    java_method(JNIEnv* env, jclass jc, const std::string& name,
                const std::string& signature) : jc(jc)
    {
        mid = env->GetMethodID(jc, name.c_str(), signature.c_str());
        if (!mid)
        {
            std::stringstream strm;
            strm << "Error GetMethodID " << name << " " << signature;
            throw std::runtime_error(strm.str());
        }
    }
    operator jmethodID()
    {
        return mid;
    }
    jclass get_class()
    {
    	return jc;
    }
private:
    jmethodID mid{};
    jclass jc;
};

class java_static_method
{
public:
	java_static_method(JNIEnv* env, jclass jc, const std::string& name,
                const std::string& signature) : jc(jc)
    {
        mid = env->GetStaticMethodID(jc, name.c_str(), signature.c_str());
        if (!mid)
        {
            std::stringstream strm;
            strm << "Error GetStaticMethodID " << name << " " << signature;
            throw std::runtime_error(strm.str());
        }
    }

    operator jmethodID()
    {
        return mid;
    }

    jclass get_class()
    {
    	return jc;
    }
private:
    jmethodID mid{};
    jclass jc;
};

int create_table()
{
    JavaVM* vm;
    JNIEnv* env;
    JavaVMInitArgs vm_args;
    jint res;
    jclass cls;
    jmethodID mid;
    jstring jstr;
    jobjectArray main_args;

    std::stringstream strm;
    strm << getenv("HBASE_HOME") << "/bin/hbase classpath";
    std::string class_path = exec(strm.str());
    strm.str("");
    strm << "-Djava.class.path=" << class_path;
    std::array<char, 32 * 1024> class_path_buf;
    strcpy(class_path_buf.data(), strm.str().c_str());

    JavaVMOption options[1];
    options[0].optionString = class_path_buf.data();

    vm_args.version = JNI_VERSION_1_8;
    vm_args.options = options;
    vm_args.nOptions = std::extent<decltype(options)>::value;
    vm_args.ignoreUnrecognized = JNI_TRUE;

    res = JNI_CreateJavaVM(&vm, (void**) &env, &vm_args);
    if (res != JNI_OK)
    {
        std::cerr << "JNI_CreateJavaVM error " << res << std::endl;
        throw std::runtime_error("JNI_CreateJavaVM error!");
        return 1;
    }

    std::vector<std::string> java_class_names{
        "org.apache.hadoop.hbase.HBaseConfiguration",
        "org.apache.hadoop.conf.Configuration",
        "org.apache.hadoop.hbase.client.ConnectionFactory",
        "org.apache.hadoop.hbase.client.Connection",
        "org.apache.hadoop.hbase.client.Admin",
        "org.apache.hadoop.hbase.HTableDescriptor",
        "org.apache.hadoop.hbase.HColumnDescriptor",
        "org.apache.hadoop.hbase.TableName"};

    std::unordered_map<std::string, jclass> java_classes;

    for (const auto& cls_name : java_class_names)
    {
        std::string class_name = cls_name;
        std::replace(class_name.begin(), class_name.end(), '.', '/');
        auto cls = env->FindClass(class_name.c_str());
        if (!cls)
        {
            std::cerr << "Check " << strm.str() << std::endl
                      << "Error calling FindClass " << class_name << std::endl;
            throw std::runtime_error(cls_name.c_str());
        }
        java_classes[cls_name] = cls;
    }

    auto HBaseConfiguration = java_classes["org.apache.hadoop.hbase.HBaseConfiguration"];
    auto Configuration = java_classes["org.apache.hadoop.conf.Configuration"];
    auto ConnectionFactory =
        java_classes["org.apache.hadoop.hbase.client.ConnectionFactory"];
    auto Connection = java_classes["org.apache.hadoop.hbase.client.Connection"];
    auto HTableDescriptor = java_classes["org.apache.hadoop.hbase.HTableDescriptor"];

    auto HBaseConfiguration_create_mid = env->GetStaticMethodID(
        HBaseConfiguration, "create", "()Lorg/apache/hadoop/conf/Configuration;");
    if (!HBaseConfiguration_create_mid)
        throw std::runtime_error(
            "GetStaticMethodID org.apache.hadoop.hbase.HBaseConfiguration.Create error!");
// HBase looks for the file named "hbase-site.xml" in all of the DIRECTORIES in the classpath
    auto conf =
        env->CallStaticObjectMethod(HBaseConfiguration, HBaseConfiguration_create_mid);
    if (!conf)
    {
        throw std::runtime_error("CallStaticObjectMethod "
                                 "org.apache.hadoop.hbase.HBaseConfiguration.create "
                                 "error!");
    }

    java_static_method createConnection{env, ConnectionFactory, "createConnection",
                                        "(Lorg/apache/hadoop/conf/Configuration;)Lorg/"
                                        "apache/hadoop/hbase/client/Connection;"};
    auto conn = env->CallStaticObjectMethod(ConnectionFactory, createConnection, conf);
    if (!conn)
    {
        throw std::runtime_error("CallStaticObjectMethod createConnection error!");
    }

    java_method getAdmin{env, Connection, "getAdmin",
                         "()Lorg/apache/hadoop/hbase/client/Admin;"};
    auto admin = env->CallObjectMethod(conn, getAdmin);
    if (!admin)
        throw std::runtime_error("CallObjectMethod getAdmin error!");

    java_static_method valueOf{env, java_classes["org.apache.hadoop.hbase.TableName"],
                               "valueOf",
                               "(Ljava/lang/String;)Lorg/apache/hadoop/hbase/TableName;"};
    auto table_name_obj = env->CallStaticObjectMethod(valueOf.get_class(), valueOf,
                                                  env->NewStringUTF(table_name.c_str()));
    if (!table_name_obj)
        throw std::runtime_error("TableName.valueOf(args[0]) error!");

    java_method HTableDescriptor_ctor{
        env, java_classes["org.apache.hadoop.hbase.HTableDescriptor"], "<init>",
        "(Lorg/apache/hadoop/hbase/TableName;)V"};

    jobject htable = env->NewObject(HTableDescriptor_ctor.get_class(),
                                    HTableDescriptor_ctor, table_name_obj);
    if (!htable)
        throw std::runtime_error("HTableDescriptor htable = new "
                                 "HTableDescriptor(TableName.valueOf(args[0])); error!");
    //    Configuration config = HBaseConfiguration.create();
    java_method HColumnDescriptor_ctor{
        env, java_classes["org.apache.hadoop.hbase.HColumnDescriptor"], "<init>",
        "(Ljava/lang/String;)V"};
    jobject col_descriptor =
        env->NewObject(HColumnDescriptor_ctor.get_class(), HColumnDescriptor_ctor,
                       env->NewStringUTF(column_family.c_str()));
    java_method addFamily{env, java_classes["org.apache.hadoop.hbase.HTableDescriptor"],
                          "addFamily",
                          "(Lorg/apache/hadoop/hbase/HColumnDescriptor;)Lorg/apache/"
                          "hadoop/hbase/HTableDescriptor;"};
    auto table_desc = env->CallObjectMethod(htable, addFamily, col_descriptor);
    if (!table_desc)
        throw std::runtime_error("CallObjectMethod addFamily error!");

    java_method createTable{env, java_classes["org.apache.hadoop.hbase.client.Admin"],
                            "createTable",
                            "(Lorg/apache/hadoop/hbase/client/TableDescriptor;)V"};

    env->CallVoidMethod(admin, createTable, htable);

    vm->DestroyJavaVM();
}

template <typename Func>
void check_duration(Func ff)
{
    auto t1 = std::chrono::high_resolution_clock::now();
    ff();
    auto t2 = std::chrono::high_resolution_clock::now();
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    std::cout << "it took " << int_ms << " milliseconds to call "
              << boost::core::demangle(typeid(Func).name()) << std::endl;
}

int main(int argc, char **argv)
{
    if (!getenv("JAVA_HOME"))
    	throw std::runtime_error("missing JAVA_HOME!");
    if (!getenv("HBASE_HOME"))
    	throw std::runtime_error("missing HBASE_HOME!");

//    create_table();
//    return 0;

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
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);
    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 0;
    }
    try
    {
        if (vm.count("list"))
            list_hbase_tables();
        if (vm.count("put"))
        {
        	auto t1 = std::chrono::high_resolution_clock::now();
            write_hbase();
            auto t2 = std::chrono::high_resolution_clock::now();
            auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

            std::cout << "it took " << int_ms
                      << " milliseconds to call write_hbase\n";

        }
        scan = vm.count("scan");
        {
            check_duration(read_hbase);
        }
    }
    catch (const apache::thrift::TException& tx)
    {
        std::cerr << "ERROR(exception): " << tx.what() << std::endl;
        std::cout << "Make sure hbase thrift2 server is started" << std::endl;
        std::cout << "~/bin/hbase-1.3.0/bin/hbase thrift2 -threadpool start "
                  << std::endl;
    }
    std::cout << get_self_path() << " pid " << getpid() << " exit. Built with g++ "
              << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__
              << std::endl;
    return 0;
}  
