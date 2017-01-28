#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/unordered_map.hpp>
#include <boost/program_options.hpp>
#include <thread>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

typedef boost::unordered_map< size_t,
                          int,
                          boost::hash<size_t>,
                          std::equal_to<size_t>,
						  boost::interprocess::allocator< std::pair<size_t, int>,
						  boost::interprocess::managed_shared_memory::segment_manager > > shared_hashmap;

size_t loops = 1000;

template<typename HashMapType>
void hashmap_perf(std::vector<size_t>& randominput, HashMapType& hashmapinst) {
    for(size_t i=0; i<loops; i++) {
    	for(auto data: randominput) {
    		hashmapinst[data]++;
    	}
    }
}
struct data {
  data() {
    std::cout << __FUNCTION__ << " tid " << gettid() << std::endl;
  }
  ~data() {
    std::cout << __FUNCTION__ << " tid " << gettid() << std::endl;
  }
};
void thread_proc(std::vector<size_t>& randominput) {
    thread_local boost::unordered_map<size_t, int> threadlocalhash;
    static thread_local data thread_local_data;
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        hashmap_perf(randominput, threadlocalhash);
        auto t2 = std::chrono::high_resolution_clock::now();
        // integral duration: requires duration_cast
        auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
        // fractional duration: no duration_cast needed
        std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
        std::cout << "HashMap in thread_local took " << fp_ms.count() << " ms, or " << int_ms.count() << " whole milliseconds\n";
    }
}

const char* get_self_path() {
  static char selfpath[PATH_MAX];
  if(selfpath[0])
    return selfpath;
  char path[PATH_MAX];
  struct stat info;
  pid_t pid = getpid();
  sprintf(path, "/proc/%d/exe", pid);
  if (readlink(path, selfpath, PATH_MAX) == -1)
    perror("readlink");
    return selfpath;
}

int main(int argc, char* argv[])
{
	boost::program_options::variables_map vm;
    boost::program_options::options_description desc("Allowed options");
	desc.add_options()("help,h", "produce help message")
	("dump", "Display content in shared hashmap")
	("remove", "Remove shared hashmap")
	  ;
	boost::program_options::store(
			boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);
	if(vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}

    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<size_t> randominput(8096);
    std::uniform_int_distribution<> dis(1, randominput.size()*16);
    for(int i=0; i<randominput.size(); i++)
    	randominput[i] = dis(gen);

	if(vm.count("remove")) {
		boost::interprocess::shared_memory_object::remove("MySharedMemory");
		return 0;
	}
	if(vm.count("dump")) {
	    boost::interprocess::managed_shared_memory segment(boost::interprocess::open_or_create, "MySharedMemory", randominput.size()*128);
	        auto thm_ = segment.find<shared_hashmap>("TestHashMap").first;
	        shared_hashmap& hashmap=*thm_;
	        size_t maxcnt = 32;
	        for(auto pair: *thm_) {
	        	if(maxcnt--) {
	        		std::cout << pair.first << " value is " << pair.second << std::endl;
	        	}
	        	else
	        		break;
	        }
		return 0;
	}
    boost::interprocess::managed_shared_memory segment(boost::interprocess::open_or_create, "MySharedMemory", randominput.size()*128);
        auto thm_ = segment.construct<shared_hashmap>("TestHashMap")(3, boost::hash<size_t>(), std::equal_to<size_t>(),
                segment.get_allocator<std::pair<size_t, int>>());

//        for(int i=0; i<1000; i++) {
//        	for(auto data: randominput) {
//        		thm_->operator[](data)++;
//        	}
//        }

        std::cout << get_self_path() << " pid " << getpid()<< " thread id " << gettid()<< " access hashmap " << loops*randominput.size() << " times. ";
        std::cout << "Built with g++ "<< __GNUC__<<"." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << std::endl;
        boost::unordered_map<size_t, int> localhash;
        {
            auto t1 = std::chrono::high_resolution_clock::now();
            hashmap_perf(randominput, *thm_);
            auto t2 = std::chrono::high_resolution_clock::now();
            // integral duration: requires duration_cast
            auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
            // fractional duration: no duration_cast needed
            std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
            std::cout << "HashMap in shared memory took " << fp_ms.count() << " ms, or " << int_ms.count() << " whole milliseconds\n";
        }
        {
            auto t1 = std::chrono::high_resolution_clock::now();
            hashmap_perf(randominput, localhash);
            auto t2 = std::chrono::high_resolution_clock::now();
            // integral duration: requires duration_cast
            auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
            // fractional duration: no duration_cast needed
            std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
            std::cout << "HashMap in local memory took " << fp_ms.count() << " ms, or " << int_ms.count() << " whole milliseconds\n";
        }
        std::thread thread1(thread_proc, std::ref(randominput));
        thread1.join();
        std::cout << argv[0] << " done\n";
    return 0;
}
//$ g++ -std=c++11 -o interprocess_hash -pthread -lrt -L/usr/local/lib interprocess_hash.cpp -lboost_program_options
// Windows 7 result
//$ ./interprocess_hash.exe
//./interprocess_hash access hashmap 8096000 times.
//Built with g++ 5.4.0
//HashMap in shared memory took 2176.78 ms, or 2176 whole milliseconds
//HashMap in local memory took 890.613 ms, or 890 whole milliseconds
//CentOS 7 guest OS, Windows 7 host OS
//./interprocess_hash access hashmap 8096000 times.
//Built with g++ 4.8.5
//HashMap in shared memory took 3098.94 ms, or 3098 whole milliseconds
//HashMap in local memory took 1097.29 ms, or 1097 whole milliseconds

//$ ./interprocess_hash.exe --remove && ./interprocess_hash.exe && ./interprocess_hash.exe --dump
//./interprocess_hash access hashmap 8096000 times.
//Built with g++ 5.4.0
//HashMap in shared memory took 2127.77 ms, or 2127 whole milliseconds
//HashMap in local memory took 892.613 ms, or 892 whole milliseconds
//89377 value is 1000
//5125 value is 1000
//77668 value is 1000
//112764 value is 1000
// [onega@localhost interprocess_hash]$ g++ -std=c++11 -o interprocess_hash -pthread -lrt -L/usr/local/lib interprocess_hash.cpp -lboost_program_options
// [onega@localhost interprocess_hash]$ ./interprocess_hash --remove && ./interprocess_hash
// /home/onega/github/onegaz/toybox/interprocess_hash/interprocess_hash pid 22059 thread id 22059 access hashmap 8096000 times. Built with g++ 4.8.5
// HashMap in shared memory took 3124.84 ms, or 3124 whole milliseconds
// HashMap in local memory took 1077.28 ms, or 1077 whole milliseconds
// data tid 22060
// HashMap in thread_local took 1149.49 ms, or 1149 whole milliseconds
// ~data tid 22060
// ./interprocess_hash done
