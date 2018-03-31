#include <unistd.h>
#include <cinttypes>
#include <cstring>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <memory>
#include <array>
#include <sstream>
#include <cds/opt/hash.h>
#include <cds/container/cuckoo_map.h>
#include <cds/memory/pool_allocator.h>
#include <cds/init.h>
#include <sys/types.h>
#include <boost/program_options.hpp>
#include <boost/core/demangle.hpp>

struct hash1
{
    size_t operator()(std::uint64_t const& s) const
    {
        return m_hash(s);
    }
    static std::hash<std::uint64_t> m_hash;
};
std::hash<std::uint64_t> hash1::m_hash;

struct hash2
{
    size_t operator()(std::uint64_t const& s) const
    {
        size_t h = ~(m_hash(s));
        return ~h + 0x9e3779b9 + (h << 6) + (h >> 2);
    }
    static std::hash<std::uint64_t> m_hash;
};
std::hash<std::uint64_t> hash2::m_hash;

template<size_t N>
struct dummytype
{
	char data[N];
};

struct count_tracker
{
	std::atomic<std::uint64_t> total_alloc{0};
	std::atomic<std::uint64_t> total_free{0};
	size_t m_size;
	void track_alloc(size_t n)
	{
		total_alloc.fetch_add(n);
	}
	void track_free(size_t n)
	{
		total_free.fetch_add(n);
	}
	std::string to_string()
	{
		std::stringstream strm;
		strm << "size " << m_size << " allocated " << total_alloc
				<< ", free " << total_free;
		return strm.str();
	}
	count_tracker(size_t N) : m_size(N)
	{

	}
	~count_tracker()
	{
		std::cout << to_string() << "\n";
	}
};

template<size_t N>
struct mem_size_tracker
{
	static count_tracker tracker;
	static void track_alloc(size_t n)
	{
		tracker.track_alloc(n);
	}
	static void track_free(size_t n)
	{
		tracker.track_free(n);
	}
	static std::string to_string()
	{
		return tracker.to_string();
	}
};
template<size_t N>
count_tracker mem_size_tracker<N>::tracker(N);

template<size_t N, size_t M>
struct pre_buffer
{
	std::array<dummytype<N>, M> preallocated;
	std::atomic<std::uint64_t> total_alloc{0};
	std::atomic<std::uint64_t> total_free{0};
	std::string to_string()
	{
		std::stringstream strm;
		strm << "size " << N << " allocated " << total_alloc << ", free " << total_free;
		return strm.str();
	}
};

pre_buffer<16, 16*1024*1024> preallocated16;
pre_buffer<8, 40*1024*1024> preallocated8;

template <typename T>
class my_allocator: public std::allocator<T>
{
public:
	typedef size_t size_type;
	typedef T* pointer;

	template<typename _Tp1>
	struct rebind
	{
		typedef my_allocator<_Tp1> other;
	};

	template <typename PreType>
	pointer from_pre_allocate(PreType& pre, size_type n, const void* hint = 0)
	{
		auto after = pre.total_alloc.fetch_add(n);
		if (after + n >= pre.preallocated.size())
		{
			return (pointer) std::allocator<T>::allocate(n, hint);
		}

		return (pointer)(&pre.preallocated[after]);
	}

	pointer allocate(size_type n, const void *hint=0)
	{

		mem_size_tracker<sizeof(T)>::track_alloc(n);

		if ((sizeof(T)%16) == 0)
		{
			return from_pre_allocate(preallocated16, n*sizeof(T)/16, hint);
		}
		if ((sizeof(T)%8) == 0)
		{
			return from_pre_allocate(preallocated8, n*sizeof(T)/8, hint);
		}
		return (pointer)std::allocator<T>::allocate(n, hint);
	}

	void deallocate(pointer p, size_type n)
	{
		mem_size_tracker<sizeof(T)>::track_free(n);
	}

};

template<typename T>
struct pool_accessor {
	typedef T   value_type;
	typedef my_allocator<T> pool_type;

    pool_type& operator()() const
    {
    	static pool_type thePool;
        return thePool;
    }
};

struct pool_traits: public cds::container::cuckoo::traits
{
	typedef std::equal_to< std::uint64_t > equal_to;
	typedef cds::opt::hash_tuple< hash1, hash2 > hash;
	typedef cds::memory::pool_allocator< dummytype<24>, pool_accessor<dummytype<24>> > allocator;
//	typedef cds::container::cuckoo::vector<4> probeset_type;
	static bool const store_hash = false;
};

struct alloc_traits: public cds::container::cuckoo::traits
{
	typedef std::equal_to< std::uint64_t > equal_to;
	typedef cds::opt::hash_tuple< hash1, hash2 > hash;
//	typedef std::allocator<int> allocator;
	typedef my_allocator<int> allocator;
//	typedef cds::container::cuckoo::vector<4> probeset_type;
	static bool const store_hash = false;
};

//typedef cds::container::CuckooMap< std::uint64_t, std::uint64_t, pool_traits > my_cuckoo_map;
typedef cds::container::CuckooMap< std::uint64_t, std::uint64_t, alloc_traits > my_cuckoo_map;

using namespace std;
struct work_thread_static
{
    my_cuckoo_map cmap;
    std::atomic_bool stop_flag;
    std::atomic<std::uint64_t> total_success;
    std::atomic<std::uint64_t> total_fail;
    std::atomic<std::uint64_t> total_write;
    std::atomic<std::uint64_t> started_threads_num;
    std::mutex cv_mutex;
    std::condition_variable cv;
    std::vector<std::uint64_t> rand_numbers;
    std::string to_string()
    {
    	std::stringstream strm;
    	strm  << "Total write " << total_write << ", total read "
			  << total_success << ", total fail "
			  << total_fail
			  ;
    	return strm.str();
    }
};

struct work_thread
{
    std::size_t m_success_count = 0;
    std::size_t m_fail_count = 0;
    std::size_t m_write_count = 0;
    void operator()()
    {
        std::random_device rd;
        std::mt19937_64 e2(rd());
        std::uniform_int_distribution<std::uint64_t> dist(0,
        		static_data.rand_numbers.size());
        // wait for main thread to check mem usage after all threads started
        static_data.started_threads_num.fetch_add(1);
        {
            std::unique_lock<std::mutex> lk(static_data.cv_mutex);
            static_data.cv.wait(lk);
        }

        while (true)
        {
            std::size_t pos = dist(e2);
            auto key = static_data.rand_numbers[pos];

            if (!static_data.cmap.contains(key))
            {
                ++m_fail_count;
                if ((m_fail_count + m_success_count) % 100 > 92)
                {
                	static_data.cmap.insert(key, key + 1);
                    ++m_write_count;
                }
            }
            else
                ++m_success_count;

            if (static_data.stop_flag.load())
                break;
        }
        static_data.total_success.fetch_add(m_success_count);
        static_data.total_fail.fetch_add(m_fail_count);
        static_data.total_write.fetch_add(m_write_count);
        // wait for main thread to check mem usage before any threads exit
        static_data.started_threads_num.fetch_sub(1);
        {
            std::unique_lock<std::mutex> lk(static_data.cv_mutex);
            static_data.cv.wait(lk);
        }
    }
    static work_thread_static static_data;

    static void init(std::size_t rand_count)
    {
    	static_data.stop_flag.store(false);
    	static_data.rand_numbers.resize(rand_count);
        std::random_device rd;
        std::mt19937_64 e2(rd());
        std::uniform_int_distribution<std::uint64_t> dist(std::llround(std::pow(2, 50)),
                                                          std::llround(std::pow(2, 54)));
        for (std::size_t n = 0; n < rand_count; ++n)
        {
        	static_data.rand_numbers[n] = dist(e2);
        }
    }
};
work_thread_static work_thread::static_data;
std::size_t rand_count = 20000000;
std::size_t thread_num = 10;
std::size_t duration_milliseconds = 10000;

void add_options(boost::program_options::options_description& desc)
{
    desc.add_options()
    	("help,h", "Produce help message")
    	("threads,t", boost::program_options::value<std::size_t>(&thread_num),"specify number of work threads")
		("duration,d", boost::program_options::value<std::size_t>(&duration_milliseconds),"specify number of milliseconds to do read/write")
		("rcount,r", boost::program_options::value<std::size_t>(&rand_count),"specify number of random numbers")
		;
}

int main(int argc, const char** argv)
{
    char buf[256];
    sprintf(buf, R"(grep -iE "VmPeak|VmSize|VmHWM|VmRSS|VmData" /proc/%d/status)", getpid());
    sprintf(buf, R"(grep -iE "VmPeak|VmSize|VmData" /proc/%d/status)", getpid());
    sprintf(buf, R"(ps -o rss,vsz,pid -p %d)", getpid());
    
    boost::program_options::variables_map vm;
    boost::program_options::options_description desc("Allowed options");
    add_options(desc);
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        std::cout << "Build with Compiler: " << BOOST_COMPILER << std::endl
                  << "Platform: " << BOOST_PLATFORM << std::endl
                  << "Library: " << BOOST_STDLIB << std::endl
                  << "Boost " << BOOST_LIB_VERSION << std::endl;
        return 0;
    }
    
    auto cmd_res = system(buf);
    
    auto start_time = std::chrono::high_resolution_clock::now();
    work_thread::init(rand_count);
    auto end_time = std::chrono::high_resolution_clock::now();
    start_time = end_time;
	std::vector<std::thread> thread_group;
	for (std::size_t i = 0; i < thread_num; i++)
		thread_group.emplace_back(std::thread(work_thread()));

	while(work_thread::static_data.started_threads_num.load()<thread_group.size())
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	std::cout << "memory usage after all threads ready\n";
	cmd_res = system(buf);
	work_thread::static_data.cv.notify_all();
	std::this_thread::sleep_for(std::chrono::milliseconds(duration_milliseconds));

	work_thread::static_data.stop_flag.store(true);
	while(work_thread::static_data.started_threads_num.load())
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	std::cout << "memory usage after all threads done but before exit\n";
	cmd_res = system(buf);
	work_thread::static_data.cv.notify_all();

	std::for_each(thread_group.begin(), thread_group.end(), std::mem_fn(&std::thread::join));
    end_time = std::chrono::high_resolution_clock::now();
    auto used_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout <<boost::core::demangle(typeid(work_thread::static_data.cmap).name())
    		  << " performance test used " << used_ms << " milliseconds."
			  << std::endl;
    std::cout << work_thread::static_data.to_string()
              << std::endl;
    std::cout << "thread_num " << thread_num << ", duration "
              << duration_milliseconds << " milliseconds, rand_count " << rand_count
              << std::endl;
//    cds::Terminate() ;
    cmd_res = system(buf);
    std::cout << std::endl;
    return 0;
}

const char* mynotes=R"mynotes(
This program is is for performance test of CuckooMap.
It starts 10 threads, print memory usage after 10 threads are ready,
then main thread notify 10 work threads to continue,
main thread wait for 10 seconds, signal work threads to stop,
after all work threads got stop signal from main thread, they pause,
main thread check memory usage again, then signal all threads to exit.

typedef cds::container::CuckooMap< std::uint64_t, std::uint64_t, alloc_traits > my_cuckoo_map;
./custom_alloc 
  RSS    VSZ   PID
 3972 607904 27723
memory usage after all threads ready
  RSS    VSZ   PID
161952 846116 27723
memory usage after all threads done but before exit
  RSS    VSZ   PID
331040 846116 27723
cds::container::CuckooMap<unsigned long, unsigned long, alloc_traits> performance test used 10023 milliseconds.
Total write 1645766, total read 1025282, total fail 23519395
thread_num 10, duration 10000 milliseconds, rand_count 20000000
  RSS    VSZ   PID
332096 796940 27723

size 16 allocated 8388544, free 4194272
size 24 allocated 1645766, free 0


typedef cds::container::CuckooMap< std::uint64_t, std::uint64_t, pool_traits > my_cuckoo_map;
./custom_alloc 
  RSS    VSZ   PID
 4080 607904 27663
memory usage after all threads ready
  RSS    VSZ   PID
162068 846116 27663
memory usage after all threads done but before exit
  RSS    VSZ   PID
351244 846116 27663
cds::container::CuckooMap<unsigned long, unsigned long, pool_traits> performance test used 10022 milliseconds.
Total write 1584952, total read 948208, total fail 22641791
thread_num 10, duration 10000 milliseconds, rand_count 20000000
  RSS    VSZ   PID
352192 796940 27663

size 24 allocated 9973496, free 4194272

)mynotes";
