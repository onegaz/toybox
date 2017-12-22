#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <boost/program_options.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
using namespace std;
using namespace boost::iostreams;

size_t getFilesize(const char* filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

size_t filesum_mmap(const std::string& filepath, int wait_milliseconds)
{
    auto t1 = std::chrono::high_resolution_clock::now();
    size_t filesize = getFilesize(filepath.c_str());
    int fd = open(filepath.c_str(), O_RDWR, 0);
    assert(fd != -1);
    void* mmappedData = mmap(NULL, filesize, PROT_READ, MAP_SHARED, fd, 0);
    assert(mmappedData != MAP_FAILED);
    char* content = (char*) mmappedData;
    size_t sum = 0;

    for (size_t pos = 0; pos < filesize; ++pos)
        sum += (unsigned int) content[pos];

    auto t2 = std::chrono::high_resolution_clock::now();
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    std::cout << "Sum is " << sum << " for " << filepath << std::endl;
    std::cout << __func__ << " used " << int_ms.count() << " milliseconds\n";

    if (wait_milliseconds>0)
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_milliseconds));

    int rc = munmap(mmappedData, filesize);
    assert(rc == 0);
    close(fd);
    return sum;
}

size_t filesum_interprocess_file_map(const std::string& filepath, int wait_milliseconds)
{
	auto t1 = std::chrono::high_resolution_clock::now();
	using namespace boost::interprocess;
	file_mapping m_file(filepath.c_str(), read_write);
    mapped_region region(m_file, read_only);

    void * addr       = region.get_address();
    std::size_t filesize  = region.get_size();
    char* content = (char*) addr;
    size_t sum = 0;

    for (size_t pos = 0; pos < filesize; ++pos)
        sum += (unsigned int) content[pos];

    auto t2 = std::chrono::high_resolution_clock::now();
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    std::cout << "Sum is " << sum << " for " << filepath << std::endl;
    std::cout << __func__ << " used " << int_ms.count() << " milliseconds\n";

    if (wait_milliseconds>0)
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_milliseconds));

    return sum;
}

size_t filesum_iostreams(const std::string& filepath, int wait_milliseconds)
{
	auto t1 = std::chrono::high_resolution_clock::now();
    boost::iostreams::mapped_file_params  params;
    params.path = filepath;
    params.mode = mapped_file_base::readwrite;

    mapped_file_source file(params);

    size_t sum = 0;
    for (size_t pos = 0; pos < file.size(); ++pos)
        sum += (unsigned int) file.data()[pos];

    auto t2 = std::chrono::high_resolution_clock::now();
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    std::cout << "Sum is " << sum << " for " << filepath << std::endl;
    std::cout << __func__ << " used " << int_ms.count() << " milliseconds\n";

    if (wait_milliseconds>0)
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_milliseconds));

    file.close();

    return sum;
}

int main(int argc, char** argv) {
	auto t1 = std::chrono::high_resolution_clock::now();
    namespace po = boost::program_options;
    po::options_description desc("Options");
    desc.add_options()
      ("help,h", "Print help messages")
          ("file,f", po::value<std::string>(),  "specify file to mmap")
		  ("wait,w", po::value<int>(),  "specify number of milliseconds to wait before exit")
		  ("mmap", "use mmap")
		  ("interprocess,i", "use boost::interprocess")
	  ;
    boost::program_options::variables_map vm;

    try
    {
        po::store(
            po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(),
            vm);
        po::notify(vm);
        if (vm.count("help"))
        {
            std::cout << argv[0] << " usage:" << std::endl << desc << std::endl;
            return 0;
        }
    }
    catch (po::error& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;
        return __LINE__;
    }

    std::string filepath;
    if (vm.count("file"))
    		filepath = vm["file"].as<std::string>();

    int wait_ms = 0;
    if (vm.count("wait"))
    		wait_ms = vm["wait"].as<int>();

    if (vm.count("mmap"))
    {
        filesum_mmap(filepath, wait_ms);
        return 0;
    }

    if (vm.count("interprocess"))
    {
    		filesum_interprocess_file_map(filepath, wait_ms);
        return 0;
    }
    filesum_iostreams(filepath, wait_ms);

    return 0;
}
