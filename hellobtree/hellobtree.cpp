#include "btree_map.h" // see https://github.com/algorithm-ninja/cpp-btree
#include<boost/tokenizer.hpp>
#include <boost/type_index.hpp>
#include <iostream>
#include <fstream>
#include <deque>
#include <string>
#include <chrono>
#include <map>
#include <thread>
#include <sys/types.h>
#include <sys/syscall.h>

void readfile(const std::string& filepath, std::deque<std::string>& col) {
   std::fstream srcfile(filepath);
   while(srcfile.good()) {
        std::string line;
       std::getline(srcfile, line);
       boost::tokenizer<> tok(line);
        for(boost::tokenizer<>::iterator beg=tok.begin(); beg!=tok.end();++beg){
            col.emplace_back(*beg);
        }
   }
}

template<typename F, class ...Args1>                                                                            
void checkperf(const std::string& prompt, F& func, Args1... args) {                                                    
    auto t1 = std::chrono::high_resolution_clock::now();  
    func(args...) ;                                                                                             
    auto t2 = std::chrono::high_resolution_clock::now();                                                        
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);                               
    std::chrono::duration<double, std::milli> fp_ms = t2 - t1;                                                  
    std::cout << prompt << " took " << fp_ms.count() << " ms, or " << int_ms.count() << " whole milliseconds\n";
} 

template<int N, typename Container>
void btreebenchmark(const Container& cont) {
    using btreemap_t=btree::btree_map<std::string, int, std::less<const std::string>, std::allocator<std::pair<const std::string, int> >, N>;
    btreemap_t btreemap;
    for(auto& data:cont)
        btreemap[data]++;
    std::cout <<"sizeof btree map is " << btreemap.size() << " from " << cont.size() << " words" << std::endl;
}

template<typename InContainer, typename OutContainer>
void benchmark(const InContainer& cont) {
    OutContainer btreemap;
    // std::cout << __func__ << " in thread " << syscall(SYS_gettid) << " container pointer is " << &btreemap << std::endl;
    int loops = 252815*40/cont.size();
    loops = std::min(100, loops);
    loops = std::max(loops, 1);
    for(int i=0; i<loops; i++)
        for(auto& data:cont)
            btreemap[data]++;
    
    // std::cout << __func__ << " in thread " << syscall(SYS_gettid) <<" sizeof " 
    // << boost::typeindex::type_id_with_cvr<OutContainer>().pretty_name() << " is " << btreemap.size() 
    // << " from " << cont.size() << " words in "<< boost::typeindex::type_id_with_cvr<InContainer>().pretty_name() << std::endl;
}

template<int N>
using btreemap_t=btree::btree_map<std::string, int, std::less<const std::string>, std::allocator<std::pair<const std::string, int> >, N>;

template<typename InContainer, typename OutContainer>
void benchmarkmt(const InContainer& words, int num) {
    auto t1 = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> mythreads;
    for(int i=0; i<num; i++)
        mythreads.emplace_back(std::thread( [&]() {benchmark<decltype(words), btreemap_t<512>>(  words); } ));
    std::for_each(std::begin(mythreads), std::end(mythreads), std::mem_fn(&std::thread::join));
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "it took "<< std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " milliseconds "
    << "processing " << words.size() << " words when launching " << num << " threads accessing "
    << boost::typeindex::type_id_with_cvr<OutContainer>().pretty_name() 
    << std::endl;
}
int main(int argc, char* argv[])
{
    std::deque<std::string> words;
    if(argc>1)
        readfile(argv[1], words);
    else
        readfile(__FILE__, words);
    btreemap_t<32> btreemap32;
    btreemap_t<64> btreemap64;
    btreemap_t<128> btreemap128;
    btreemap_t<256> btreemap256;
    btreemap_t<512> btreemap512;
    std::map<std::string, int> stlmap;
    // checkperf("btreemap32", benchmark<decltype(words), decltype(btreemap32)>, words, btreemap32);
    // checkperf("btreemap64", benchmark<decltype(words), decltype(btreemap64)>, words, btreemap64);
    // checkperf("btreemap128", benchmark<decltype(words), decltype(btreemap128)>, words, btreemap128);
    // checkperf("btreemap256", benchmark<decltype(words), decltype(btreemap256)>, words, btreemap256);
    // checkperf("btreemap512", benchmark<decltype(words), decltype(btreemap512)>, words, btreemap512);
    int thread_count = 8;
    benchmarkmt<decltype(words), std::map<std::string, int>>(words, thread_count);
    benchmarkmt<decltype(words), btreemap_t<512>>(words, thread_count);
    benchmarkmt<decltype(words), std::map<std::string, int>>(words, thread_count);
    benchmarkmt<decltype(words), btreemap_t<512>>(words, thread_count);
    // auto t1 = std::chrono::high_resolution_clock::now();
    // const int NUM_THREADS=4;
    // {
    // std::thread mythreads[NUM_THREADS];
    // btreemap_t<256> btreemap256[NUM_THREADS];
    // for(int i=0; i<NUM_THREADS; i++)
    //     mythreads[i] = std::thread( [&]() {benchmark<decltype(words), btreemap_t<512>>(  words); } );
    // std::for_each(std::begin(mythreads), std::end(mythreads), std::mem_fn(&std::thread::join));
    // }
    // auto t2 = std::chrono::high_resolution_clock::now();
    // {
    // std::thread mythreads[NUM_THREADS];
    // std::map<std::string, int> stlmap[NUM_THREADS];
    // for(int i=0; i<NUM_THREADS; i++)
    //     mythreads[i] = std::thread( [&]() {benchmark<decltype(words), std::map<std::string, int>>(  words); } );
    // for(int i=0; i<NUM_THREADS; i++)
    //     if(mythreads[i].joinable())
    //         mythreads[i].join();
    // }
    // auto t3 = std::chrono::high_resolution_clock::now();
    // std::cout << "btree<256> " << " took " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " milliseconds, "
    // << "std::map " << " took " << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count() << " milliseconds" << std::endl;

    // checkperf("stlmap", benchmark<decltype(words), decltype(stlmap)>, words, stlmap);
    // checkperf("16", btreebenchmark<16, std::deque<std::string>>, words);
    // checkperf("32", btreebenchmark<32, std::deque<std::string>>, words);
    // checkperf("64", btreebenchmark<64, std::deque<std::string>>, words);
    // checkperf("128", btreebenchmark<128, std::deque<std::string>>, words);
    // checkperf("256", btreebenchmark<256, std::deque<std::string>>, words);
    // checkperf("512", btreebenchmark<256, std::deque<std::string>>, words);
    // checkperf("1024", btreebenchmark<1024, std::deque<std::string>>, words);
    // checkperf("2048", btreebenchmark<2048, std::deque<std::string>>, words);
    std::cout << argv[0] << " Built with g++ "<< __GNUC__<<"." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << std::endl;
    return 0;
}
/*
[onega@localhost ~]$ make -C /home/onega/workspace/btree
make: Entering directory `/home/onega/workspace/btree'
/home/onega/gcc-6.3.0/bin/g++ -o hellobtree -g -pthread -I cpp-btree -std=c++11 -O2 -fmax-errors=8 -Wall -Wl,-rpath=/home/onega/gcc-6.3.0/lib64 hellobtree.cpp

[onega@localhost ~]$ /home/onega/workspace/btree/hellobtree /home/onega/Downloads/pg54298.txt
it took 14360 milliseconds processing 252815 words when launching 8 threads accessing std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, int, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, int> > >
it took 13102 milliseconds processing 252815 words when launching 8 threads accessing btree::btree_map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, int, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const>, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, int> >, 512>
it took 12746 milliseconds processing 252815 words when launching 8 threads accessing std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, int, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, int> > >
it took 12674 milliseconds processing 252815 words when launching 8 threads accessing btree::btree_map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, int, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const>, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, int> >, 512>
/home/onega/workspace/btree/hellobtree Built with g++ 6.3.0


*/