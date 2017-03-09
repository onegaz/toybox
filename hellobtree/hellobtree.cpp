#include "btree_map.h" // see https://github.com/algorithm-ninja/cpp-btree
#include<boost/tokenizer.hpp>
#include <iostream>
#include <fstream>
#include <deque>
#include <string>
#include <chrono>
#include <map>

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
void benchmark(const InContainer& cont, OutContainer& btreemap) {
    for(auto& data:cont)
        btreemap[data]++;
    std::cout <<"sizeof btree map is " << btreemap.size() << " from " << cont.size() << " words" << std::endl;
}

template<int N>
using btreemap_t=btree::btree_map<std::string, int, std::less<const std::string>, std::allocator<std::pair<const std::string, int> >, N>;

int main(int argc, char* argv[])
{
    std::deque<std::string> words;
    if(argc>1)
        readfile(argv[1], words);
    btreemap_t<32> btreemap32;
    btreemap_t<64> btreemap64;
    btreemap_t<128> btreemap128;
    btreemap_t<256> btreemap256;
    btreemap_t<512> btreemap512;
    std::map<std::string, int> stlmap;
    checkperf("btreemap32", benchmark<decltype(words), decltype(btreemap32)>, words, btreemap32);
    checkperf("btreemap64", benchmark<decltype(words), decltype(btreemap64)>, words, btreemap64);
    checkperf("btreemap128", benchmark<decltype(words), decltype(btreemap128)>, words, btreemap128);
    checkperf("btreemap256", benchmark<decltype(words), decltype(btreemap256)>, words, btreemap256);
    checkperf("btreemap512", benchmark<decltype(words), decltype(btreemap512)>, words, btreemap512);
    
    checkperf("stlmap", benchmark<decltype(words), decltype(stlmap)>, words, stlmap);
    checkperf("16", btreebenchmark<16, std::deque<std::string>>, words);
    checkperf("32", btreebenchmark<32, std::deque<std::string>>, words);
    checkperf("64", btreebenchmark<64, std::deque<std::string>>, words);
    checkperf("128", btreebenchmark<128, std::deque<std::string>>, words);
    checkperf("256", btreebenchmark<256, std::deque<std::string>>, words);
    checkperf("512", btreebenchmark<256, std::deque<std::string>>, words);
    checkperf("1024", btreebenchmark<1024, std::deque<std::string>>, words);
    checkperf("2048", btreebenchmark<2048, std::deque<std::string>>, words);
    std::cout << argv[0] << " Built with g++ "<< __GNUC__<<"." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << std::endl;
    return 0;
}
/*
[onega@localhost btree]$ make
/home/onega/gcc-6.3.0/bin/g++ -o hellobtree -I cpp-btree -std=c++11 -O2 -fmax-errors=8 -Wall -Wl,-rpath=/home/onega/gcc-6.3.0/lib64 hellobtree.cpp
[onega@localhost btree]$ ./hellobtree /home/onega/Downloads/pg54298.txt
sizeof btree map is 13776 from 252815 words
btreemap32 took 93.0385 ms, or 93 whole milliseconds
sizeof btree map is 13776 from 252815 words
btreemap64 took 98.893 ms, or 98 whole milliseconds
sizeof btree map is 13776 from 252815 words
btreemap128 took 103.483 ms, or 103 whole milliseconds
sizeof btree map is 13776 from 252815 words
btreemap256 took 89.0869 ms, or 89 whole milliseconds
sizeof btree map is 13776 from 252815 words
btreemap512 took 123.56 ms, or 123 whole milliseconds
sizeof btree map is 13776 from 252815 words
stlmap took 65.4553 ms, or 65 whole milliseconds
sizeof btree map is 13776 from 252815 words
16 took 90.2602 ms, or 90 whole milliseconds
sizeof btree map is 13776 from 252815 words
32 took 91.6389 ms, or 91 whole milliseconds
sizeof btree map is 13776 from 252815 words
64 took 95.4567 ms, or 95 whole milliseconds
sizeof btree map is 13776 from 252815 words
128 took 97.0026 ms, or 97 whole milliseconds
sizeof btree map is 13776 from 252815 words
256 took 89.9757 ms, or 89 whole milliseconds
sizeof btree map is 13776 from 252815 words
512 took 95.6076 ms, or 95 whole milliseconds
sizeof btree map is 13776 from 252815 words
1024 took 91.8562 ms, or 91 whole milliseconds
sizeof btree map is 13776 from 252815 words
2048 took 79.722 ms, or 79 whole milliseconds
./hellobtree Built with g++ 6.3.0


*/