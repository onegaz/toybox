#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

using boost::asio::ip::tcp;
std::string file_path;
std::size_t wait_before_close = 5000; // milliseconds
unsigned short port = 12345;

void session(tcp::socket sock)
{
  try
  {
        std::ifstream myfile(file_path.c_str());
        if (!myfile)
        {
          std::cout << "Failed to open " << file_path << std::endl;
          return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::cout << "Start sending " << file_path << " to client\n";
        while (true)
        {
            std::string line;
            if (std::getline(myfile, line))
            {
                //strm << line;
                line.append("\n");
                boost::asio::write(sock, boost::asio::buffer(line.c_str(), line.length()));
            }
            else
                break;            
        }

        std::cout << "finished sending file " << file_path << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // wait for flink client to print something
        std::cout << "close session " << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Exception in thread: " << e.what() << "\n";
  }
}

void server(boost::asio::io_service& io_service, unsigned short port)
{
  tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
  std::cout << "list on port " << port << std::endl;
  for (;;)
  {
    tcp::socket sock(io_service);
    a.accept(sock);
    std::thread(session, std::move(sock)).detach();
  }
}

int main(int argc, char* argv[])
{
  
  try
  {
    boost::program_options::variables_map vm;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
    	("help,h", "Produce help message")
    	("port,p", boost::program_options::value<decltype(port)>(&port)->default_value(port),"specify port number")
		("file_path,f", boost::program_options::value<decltype(file_path)>(&file_path),"specify file to send back to client")
		("wait_before_close,w", boost::program_options::value<decltype(wait_before_close)>(&wait_before_close)->default_value(wait_before_close),"specify number of milliseconds to wait before disconnect with client")
		;
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("file_path")==0)
    {
      std::cout << desc << std::endl;
      return 1;
    }
    boost::asio::io_service io_context;

    server(io_context, port);
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

// g++ -std=c++11 -pthread send_file.cpp -l boost_system