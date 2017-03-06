#include <iostream>
#include "gen-cpp/flume_constants.h"
#include "gen-cpp/flume_types.h"
#include "gen-cpp/ThriftSourceProtocol.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <boost/make_shared.hpp> 
#include <sstream>
#include <fcntl.h>
#include <chrono>
#include <boost/type_index.hpp>
#include <boost/program_options.hpp>
#include <link.h>
using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
size_t loop= 2;
size_t batch_size=2;
unsigned short thrift_port = 44444;
std::string thrift_server = "127.0.0.1";
int success_cnt = 0;
int error_cnt = 0;

template<typename F, class ...Args1>                                                                            
void checkperf(const string& prompt, F& func, Args1... args) {                                                    
    auto t1 = std::chrono::high_resolution_clock::now();                                                        
    func(args...) ;                                                                                             
    auto t2 = std::chrono::high_resolution_clock::now();                                                        
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);                               
    std::chrono::duration<double, std::milli> fp_ms = t2 - t1;                                                  
    std::cout << prompt << " took " << fp_ms.count() << " ms, or " << int_ms.count() << " whole milliseconds\n";
}                                                                                                               

int try_flume() {
    boost::shared_ptr<TTransport> socket=boost::make_shared<TSocket>(thrift_server,thrift_port);
    boost::shared_ptr<TTransport> transport=boost::make_shared<TFramedTransport>(socket);
    boost::shared_ptr<TProtocol> protocol = boost::make_shared<TCompactProtocol>(transport);
    std::unique_ptr<ThriftSourceProtocolClient> flumeclient(new ThriftSourceProtocolClient(protocol));
    std::map<std::string, std::string> headers;
    headers.insert(std::make_pair("head", "head"));
    transport->open();
    if(!transport->isOpen())
    {
        std::cerr << __func__ << " failed to open transport" << std::endl;
        return -1;
    }
    Status::type res;
    std::vector<ThriftFlumeEvent> eventbatch;
    for(size_t i=0;i<loop;i++){
        eventbatch.clear();
        for(size_t j=0;j<=batch_size;j++) {
            ThriftFlumeEvent tfEvent;
            headers["headf2"] = std::to_string(i*loop*100 + j);
            tfEvent.__set_headers(headers);
            std::stringstream ss;
            ss << "flume event body for loop " << i << " step " << j;
            tfEvent.__set_body(ss.str());
            eventbatch.emplace_back(std::move(tfEvent));
        }            
        res =flumeclient->appendBatch(eventbatch);
        if(res == Status::OK){
            success_cnt++;
        }else{
            error_cnt++;
            std::cerr << __func__ << " operation " << i << " failed, error code " << res << " total error count: " << error_cnt<< std::endl;
        }   
    } 
    transport->close();
    std::cout << __func__ << " successfully finished " << success_cnt << " operations with apache flume via thrift interface" << std::endl;
    return 0;
}

static int dl_iterate_phdr_callback(struct dl_phdr_info *info, size_t size, void *data)
{
    std::cout << info->dlpi_name << "\t(" << info->dlpi_phnum << " segments)" << std::endl;
    return 0;
}

int main(int argc, char * argv[]){
    // using po=boost::program_options;
	boost::program_options::variables_map vm;
    boost::program_options::options_description desc("Allowed options");
	desc.add_options()("help,h", "produce help message")
	("loop", boost::program_options::value<size_t>(&loop)->default_value(loop), "loop count")
	("batch", boost::program_options::value<size_t>(&batch_size)->default_value(batch_size), "batch size, number of events sent via appendBatch")
    ("thriftserver", boost::program_options::value<decltype(thrift_server)>(&thrift_server)->default_value(thrift_server), "flume thift server address")
    ("thriftport", boost::program_options::value<decltype(thrift_port)>(&thrift_port)->default_value(thrift_port), "flume thift server port")
	  ;
	boost::program_options::store(
			boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);
	if(vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}    
    checkperf( "flume", try_flume);
    dl_iterate_phdr(dl_iterate_phdr_callback, NULL);
    return 0;
}


