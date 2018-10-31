#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <streambuf>
#include <memory>
#include <string>
#include <boost/program_options.hpp>
#include <grpcpp/grpcpp.h>

#include "api.pb.h"
#include "api.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using hellogrpc::HelloRequest;
using hellogrpc::HelloReply;
using hellogrpc::Greeter;
using grpc::Channel;
using grpc::ClientContext;

class GreeterServiceImpl final : public Greeter::Service
{
    Status SayHello(ServerContext* context, const HelloRequest* request,
                    HelloReply* reply) override
    {
        std::stringstream strm;
        strm << "Received " << request->name()
             << ", content size: " << request->content().size() << "\n";

        reply->set_message(strm.str());
        if (request->content().size())
        {
            std::ofstream ofs("test.out");
            ofs.write(request->content().data(), request->content().size());
        }
        return Status::OK;
    }
};

void RunServer(int port)
{
    std::string server_address("0.0.0.0:" + std::to_string(port));
    GreeterServiceImpl service;
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();
}

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
    std::string server_address;
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
			"program mode, server: 0, client: 1")
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

    GreeterClient greeter(grpc::CreateChannel("localhost:" + std::to_string(port),
                                              grpc::InsecureChannelCredentials()));
    std::string reply = greeter.SayHello(filepath, filepath);
    std::cout << reply << std::endl;

    return 0;
}
