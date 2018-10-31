#include <atomic>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <streambuf>
#include <memory>
#include <string>
#include <thread>
#include <boost/program_options.hpp>
#include <grpcpp/grpcpp.h>
//#include <grpc/support/log.h>
#include "api.pb.h"
#include "api.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerCompletionQueue;
using hellogrpc::HelloRequest;
using hellogrpc::HelloReply;
using hellogrpc::Greeter;
using grpc::Channel;
using grpc::ClientContext;

Status Process(const std::string& msg, const HelloRequest* request, HelloReply* reply)
{
	static std::atomic_size_t count;
	std::string saveas = "test" + std::to_string(count++)+".out";
    std::stringstream strm;
    strm << msg << " Received " << request->name()
         << ", content size: " << request->content().size() << ", save as "
		 << saveas << "\n";

    reply->set_message(strm.str());
    if (request->content().size())
    {
        std::ofstream ofs(saveas.c_str());
        if (!ofs)
        	return Status::CANCELLED; // should return some error message, see Status ctor

        ofs.write(request->content().data(), request->content().size());
        return Status::OK;
    }
    return Status::CANCELLED;
}

class GreeterServiceSyncImpl final : public Greeter::Service
{
    Status SayHello(ServerContext* context, const HelloRequest* request,
                    HelloReply* reply) override
    {
    	return Process("GreeterServiceSyncImpl ", request, reply);
    }
};

void RunServer(int port)
{
    std::string server_address("0.0.0.0:" + std::to_string(port));
    GreeterServiceSyncImpl service;
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();
}

class ServerAsyncImpl final
{
public:
    ServerAsyncImpl(int port) : port_(port)
    {
    }
    ~ServerAsyncImpl()
    {
        server_->Shutdown();
        cq_->Shutdown();
    }

    void Run()
    {
        std::string server_address("0.0.0.0:" + std::to_string(port_));

        ServerBuilder builder;

        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

        builder.RegisterService(&service_);

        cq_ = builder.AddCompletionQueue();

        server_ = builder.BuildAndStart();
        std::cout << "Server listening on " << server_address << std::endl;

        std::thread handle_rpc_thrd(&ServerAsyncImpl::HandleRpcs, this);
        handle_rpc_thrd.join();
    }

private:
    class CallData
    {
    public:
        CallData(Greeter::AsyncService* service, ServerCompletionQueue* cq) :
            service_(service),
            cq_(cq),
            responder_(&ctx_),
            status_(CREATE)
        {
            Proceed();
        }

        void Proceed()
        {
            if (status_ == CREATE)
            {

                status_ = PROCESS;

                service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_, this);
            }
            else if (status_ == PROCESS)
            {
                new CallData(service_, cq_);
                auto sts = Process("GreeterServiceSyncImpl ", &request_, &reply_);

                status_ = FINISH;
                responder_.Finish(reply_, sts, this);
            }
            else
            {
                GPR_ASSERT(status_ == FINISH);
                delete this;
            }
        }

    private:
        Greeter::AsyncService* service_;
        ServerCompletionQueue* cq_;
        ServerContext ctx_;
        HelloRequest request_;
        HelloReply reply_;

        ServerAsyncResponseWriter<HelloReply> responder_;

        enum CallStatus
        {
            CREATE,
            PROCESS,
            FINISH
        };
        CallStatus status_; // The current serving state.
    };

    void HandleRpcs()
    {
        new CallData(&service_, cq_.get());
        void* tag = nullptr;
        bool ok = true;
        while (ok)
        {
            GPR_ASSERT(cq_->Next(&tag, &ok));
            if(ok)
            	static_cast<CallData*>(tag)->Proceed();
        }
    }

    int port_ = 50051;
    std::unique_ptr<ServerCompletionQueue> cq_;
    Greeter::AsyncService service_;
    std::unique_ptr<Server> server_;
};

class GreeterClient
{
public:
    GreeterClient(std::shared_ptr<Channel> channel) : stub_(Greeter::NewStub(channel))
    {
    }

    std::string SayHello(const std::string& user, const std::string& filepath)
    {
        HelloRequest request;
        request.set_name(user);

        std::ifstream ifs(filepath.c_str());
        if (!ifs)
            std::cerr << "Failed to open " << filepath << "\n";

        std::string str((std::istreambuf_iterator<char>(ifs)),
                        std::istreambuf_iterator<char>());
        request.set_content(str);
        HelloReply reply;

        ClientContext context;

        Status status = stub_->SayHello(&context, request, &reply);

        if (status.ok())
        {
            return reply.message();
        }
        else
        {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
            return "RPC failed";
        }
    }

private:
    std::unique_ptr<Greeter::Stub> stub_;
};

int main(int argc, char** argv)
{
    int port = 50051;
    int mode = 0;
    std::string server_address = "localhost";
    std::string filepath;
    // clang-format off
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message")
		("port",
			boost::program_options::value<decltype(port)>(&port)->default_value(port),
			"Port number to connect")
		("server",
			boost::program_options::value<decltype(server_address)>(&server_address)->default_value(server_address),
			"rpc Server address")
		("mode", boost::program_options::value<decltype(mode)>(&mode)->default_value(mode),
			"program mode, server: 0, client: 1, async server: 2")
		("filepath",
			boost::program_options::value<decltype(filepath)>(&filepath)->default_value(filepath),
			"filepath")
			;
    // clang-format on

    boost::program_options::variables_map vm;
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);
    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 0;
    }

    if (mode == 0)
    {
        RunServer(port);
        return 0;
    }

    if (mode == 2)
    {
        ServerAsyncImpl server(port);
        server.Run();
    }
    GreeterClient greeter(grpc::CreateChannel(server_address + ":" + std::to_string(port),
                                              grpc::InsecureChannelCredentials()));
    std::string reply = greeter.SayHello(filepath, filepath);
    std::cout << reply << std::endl;

    return 0;
}
