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
using grpc::CompletionQueue;
using grpc::ClientAsyncResponseReader;

Status Process(const std::string& msg, const HelloRequest* request, HelloReply* reply)
{
    static std::atomic_size_t count;
    std::string saveas = "test" + std::to_string(count++) + ".out";
    std::stringstream strm;
    strm << msg << " Received " << request->name()
         << ", content size: " << request->content().size() << ", save as " << saveas
         << "\n";

    reply->set_message(strm.str());
    if (request->content().size())
    {
        std::ofstream ofs(saveas.c_str());
        if (!ofs)
            return Status(grpc::StatusCode::INTERNAL, "Failed to open file");

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

class GreeterSyncClient
{
public:
    GreeterSyncClient(std::shared_ptr<Channel> channel) : stub_(Greeter::NewStub(channel))
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

class GreeterAsyncClient
{
public:
    explicit GreeterAsyncClient(std::shared_ptr<Channel> channel) :
        stub_(Greeter::NewStub(channel))
    {
    }

    void SayHello(const std::string& user, const std::string& filepath)
    {
        HelloRequest request;
        request.set_name(user);

        std::ifstream ifs(filepath.c_str());
        if (!ifs)
            std::cerr << "Failed to open " << filepath << "\n";

        std::string str((std::istreambuf_iterator<char>(ifs)),
                        std::istreambuf_iterator<char>());
        request.set_content(str);

        AsyncClientCall* call = new AsyncClientCall;
        call->response_reader =
            stub_->PrepareAsyncSayHello(&call->context, request, &cq_);
        call->response_reader->StartCall();
        call->response_reader->Finish(&call->reply, &call->status, (void*) call);
    }

    void AsyncCompleteRpc1()
    {
        void* got_tag = nullptr;
        bool ok = false;
    	CompletionQueue::NextStatus got = CompletionQueue::NextStatus::TIMEOUT;
    	gpr_timespec deadline;
    	deadline.clock_type = GPR_TIMESPAN;
    	deadline.tv_sec = 0;
    	deadline.tv_nsec = 10000000;

    	got = cq_.AsyncNext<gpr_timespec>(&got_tag, &ok, deadline);
    }

    void AsyncCompleteRpc()
    {
        void* got_tag = nullptr;
        bool ok = false;

        while (cq_.Next(&got_tag, &ok))
        {
            AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);
            GPR_ASSERT(ok);
            if (call->status.ok())
                std::cout << "Greeter received: " << call->reply.message() << std::endl;
            else
                std::cout << "RPC failed" << std::endl;
            delete call;
        }
        std::cout << "RPC shutdown" << std::endl;
    }

    void Stop()
    {
    	cq_.Shutdown();
    }

private:
    struct AsyncClientCall
    {
        HelloReply reply;
        ClientContext context;
        Status status;
        std::unique_ptr<ClientAsyncResponseReader<HelloReply>> response_reader;
    };

    std::unique_ptr<Greeter::Stub> stub_;

    CompletionQueue cq_;
};

int main(int argc, char** argv)
{
    int port = 50051;
    int mode = 0;
    int request_num = 2;
    std::string server_address = "localhost";
    std::string filepath;
    // clang-format off
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message")
		("port",
			boost::program_options::value<decltype(port)>(&port)->default_value(port),
			"Port number to connect")
		("request_num",
			boost::program_options::value<decltype(request_num)>(&request_num)->default_value(request_num),
			"Number of requests send by client")
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

    std::string server_end_point = server_address + ":" + std::to_string(port);
    if (mode == 3)
    {
        GreeterSyncClient greeter(grpc::CreateChannel(server_end_point,
                                                  grpc::InsecureChannelCredentials()));
        std::string reply = greeter.SayHello(filepath, filepath);
        std::cout << reply << std::endl;

    }

    if (mode == 4)
    {
        GreeterAsyncClient greeter(
            grpc::CreateChannel(server_end_point, grpc::InsecureChannelCredentials()));

        std::thread thread_(&GreeterAsyncClient::AsyncCompleteRpc, &greeter);

        for (int i = 0; i < request_num; i++)
        {
            greeter.SayHello(filepath, filepath);
        }

        std::cout << "Stopping GreeterAsyncClient..." << std::endl << std::endl;
        greeter.Stop();
        thread_.join();
    }
    return 0;
}
