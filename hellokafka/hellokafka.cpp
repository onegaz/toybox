// based on https://github.com/danieljoos/libkafka-asio
// examples/fetch_cxx11.cpp

#include <iostream>
#include <boost/asio.hpp>
#include <libkafka_asio/libkafka_asio.h>
#include <thread>
#include <chrono>
#include <future>
#include <sstream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <ctime>
#include <unistd.h>
#include <boost/optional.hpp>
#include <atomic>
#include <boost/program_options.hpp>

#define gettid() syscall(SYS_gettid)
std::string server_address="localhost";
unsigned short port=9092;
int produce_count=0;
int producer_timeout_seconds = 5;
int producer_interval = 500;
int consumer_idle_exit = 10;

void produce_messages(libkafka_asio::Connection& connection, const std::string& topic,
                      int count, std::promise<int>* promObj)
{
    if (count < 1)
    {
        promObj->set_value(0);
        return;
    }

    std::atomic<size_t> message_count;
    auto get_current_datetime_str = []() {
        time_t rawtime;
        struct tm* timeinfo;
        char buffer[80];
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buffer, 80, "%02m-%02d-%Y %I:%M:%S", timeinfo);
        return std::string(buffer);
    };

    for (int i = 0; i < count; i++)
    {
        std::stringstream ss;
        ss << "message from " << __func__ << " in thread " << gettid() << " at "
           << get_current_datetime_str();
        libkafka_asio::ProduceRequest request;
        request.AddValue(ss.str(), topic, 0);
        std::this_thread::sleep_for(std::chrono::microseconds(producer_interval));
        connection.AsyncRequest(
            request, [&](const libkafka_asio::Connection::ErrorCodeType& err,
                         const libkafka_asio::ProduceResponse::OptionalType&
                             response) { // it is running in io_service thread
                if (err)
                {
                    std::cerr << "Error: " << boost::system::system_error(err).what()
                              << std::endl;
                    promObj->set_value(-1);
                    message_count.fetch_add(count + 1);
                    return;
                }
                message_count.fetch_add(1);
                std::cout << "tid " << gettid() << " Successfully produced message! "
                          << message_count.load() << std::endl;
                if (message_count.load() >= count)
                {
                    promObj->set_value(0);
                }
            });
    }

    for (int i = 0; i < producer_timeout_seconds * 1000; i++)
    {
        if (message_count.load() >= count)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << __func__ << " exit from tid " << gettid() << " message_count is "
              << message_count.load() << std::endl;
}

const char* get_self_path()
{
    static char selfpath[PATH_MAX];
    if (selfpath[0])
        return selfpath;
    char path[PATH_MAX];
    struct stat info;
    pid_t pid = getpid();
    sprintf(path, "/proc/%d/exe", pid);
    if (readlink(path, selfpath, PATH_MAX) == -1)
        perror("readlink");
    return selfpath;
}

int main(int argc, char** argv)
{
	std::string topic = "test";
    boost::asio::io_service ios;
    boost::optional<boost::asio::io_service::work> work_io_service_guard(std::ref(ios));
    boost::program_options::options_description desc("Allowed options");
    // clang-format off
    desc.add_options()("help,h", "produce help message")(
        "port", boost::program_options::value<decltype(port)>(&port)->default_value(port),
        "Port number to connect")
		("server",
        boost::program_options::value<decltype(server_address)>(&server_address)
            ->default_value(server_address), "Kafka Server address")
		("topic",
		boost::program_options::value<decltype(topic)>(&topic)
			->default_value(topic), "Kafka topic")
		("producer_interval",
		boost::program_options::value<decltype(producer_interval)>(&producer_interval)
			->default_value(producer_interval), "producer_interval in milliseconds")
		("produce_count",
        boost::program_options::value<int>(&produce_count)->default_value(produce_count),
        "Produce specified number of messages, default is 0")(
        "producer_timeout",
        boost::program_options::value<int>(&producer_timeout_seconds)
            ->default_value(producer_timeout_seconds),
        "Producer timeout in 5 seconds by default")(
        "consumer_idle_exit",
        boost::program_options::value<int>(&consumer_idle_exit)
            ->default_value(consumer_idle_exit),
        "Consumer will stop after being idle for specified time, default is 10ms");
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

    std::thread run_thread([&] {
        ios.run();
        std::cout << "boost::asio thread " << gettid() << " exit\n";
    });

    std::string broker_address =
        server_address + ":" + std::to_string(port); // "localhost:9092"
    libkafka_asio::Connection::Configuration configuration;
    configuration.auto_connect = true;
    configuration.client_id = "hellokafka";
    configuration.socket_timeout = 10000;
    configuration.SetBrokerFromString(broker_address);
    libkafka_asio::Connection connection(ios, configuration);
    // try: bin/kafka-console-consumer.sh --bootstrap-server localhost:9092 --topic test
    // --from-beginning
    libkafka_asio::FetchRequest request;

    int partition = 0;
    request.FetchTopic(topic, partition, 0); //
    // Helper to interpret the received bytes as string
    auto BytesToString = [](const libkafka_asio::Bytes& bytes) -> std::string {
        if (!bytes || bytes->empty())
        {
            return "";
        }
        return std::string((const char*) &(*bytes)[0], bytes->size());
    };
    std::promise<int> promiseObj;
    std::future<int> futureObj = promiseObj.get_future();
    std::thread producer_thread(produce_messages, std::ref(connection), std::ref(topic),
                                produce_count, &promiseObj);
    std::cout << futureObj.get() << std::endl; // wait for producer_thread finish its work
    std::atomic<int> recevied_message_count;
    std::thread consumer_thread([&] {
        connection.AsyncRequest(
            request, [&](const libkafka_asio::Connection::ErrorCodeType& err,
                         const libkafka_asio::FetchResponse::OptionalType&
                             response) { // this code is running in ios_service thread
                if (err)
                {
                    std::cerr << "Error: " << boost::system::system_error(err).what()
                              << std::endl;
                    return;
                }
                recevied_message_count++;
                std::cout << "Received messages in thread " << gettid() << std::endl;
                std::for_each(response->begin(), response->end(),
                              [&](const libkafka_asio::MessageAndOffset& message) {
                                  std::cout << BytesToString(message.value())
                                            << std::endl;
                              });
            });
        // exit if there is no message within specified time
        int last_count = 0;
        do
        {
            usleep(consumer_idle_exit * 1000);
            if (last_count == recevied_message_count.load())
                break;
            last_count = recevied_message_count.load();
        } while (true);
        std::cout << "consumer_thread " << gettid() << " exit\n";
    });

    producer_thread.join();
    consumer_thread.join();
    work_io_service_guard = boost::none;
    run_thread.join();
    std::cout << get_self_path() << " pid " << getpid() << " exit. Built with g++ "
              << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__
              << std::endl;
    return 0;
}

// [onega@localhost hellokafka]$ make
// g++ -o hellokafka -g -pthread -I/home/onega/src/libkafka-asio/lib -I/home/onega/src/snappy -std=c++11 hellokafka.cpp -L/home/onega/src/snappy/.libs -lsnappy -lboost_program_options -lboost_system -lz
