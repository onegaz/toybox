//============================================================================
// Name        : prime.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <cmath>
#include <boost/program_options.hpp>
#include <boost/timer/timer.hpp>
#include <boost/version.hpp>
#include <boost/config.hpp>
#include <thread>
#include <future>
using namespace std;
// https://en.wikipedia.org/wiki/Sieve_of_Sundaram
vector<int> sieve_of_sundaram(int n) {
	vector<int> all(n+1);
	for(int i=1; i<n/2; i++)
		for(int j=i; j<n/2; j++) {
			int pos = i+j+2*i*j;
			if(pos>=0 && pos<=n)
				all[pos] = 1;
			else
				break;
		}
	vector<int> result;
	if(n>=2)
		result.push_back(2);
	for(int i=1; i<n/2; i++)
		if(all[i]==0)
			result.push_back(2*i+1);
	return result;
}

bool is_prime(int n) {
	if(n<2)	return false;	// 1
	if(n<4) return true;	// 2, 3
	if((n%2)==0) return false; // odd number
	int squareroot = sqrt(n);
	for(int i=3; i<=squareroot; i+=2) {
		if((n%i) ==0)
			return false;
	}
	return true;
}

int main(int argc, char** argv) {
	int maxnum = 100;
	boost::program_options::variables_map vm;
	boost::program_options::options_description desc("Allowed options");
	desc.add_options()("help,h", "produce help message")
	("maxnum", boost::program_options::value<decltype(maxnum)>(&maxnum)->default_value(maxnum), "max number to check for prime numbers")
	("verify", "verify each prime number found")
	("show", "show all prime numbers up to maxnum")
	("parallel", "run in parallel mode")
	  ;
	boost::program_options::store(
			boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);
	if(vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}
	cout << "!!!sieve_of_sundaram!!!" << endl;
	unsigned int hardware_concurrency = std::max( (unsigned int)2, std::thread::hardware_concurrency());
	vector<int> primes;
	{
		boost::timer::auto_cpu_timer sieve_of_sundaram_timer;
		primes = sieve_of_sundaram(maxnum);
	}
	cout << "there are " << primes.size() << " prime numbers up to " << maxnum <<endl;
	if(vm.count("show"))
		std::copy(primes.begin(), primes.end(), ostream_iterator<int>(cout, " "));
	if(vm.count("verify")) {
		boost::timer::auto_cpu_timer verify_timer;
		size_t min_batch_per_thread = 32768;
		size_t thread_count = primes.size()/min_batch_per_thread;
		thread_count = std::min(thread_count, hardware_concurrency);
		size_t thread_load = primes.size()/std::max( static_cast<size_t>(1), thread_count);
		auto check_prime_func=[=](const vector<int>& primes, size_t start) {
			for(size_t i=0; i<thread_load; i++) {
				if(i+start>=primes.size()) {
					break;
				}
				if(is_prime(primes[i+start])==false)
					cout << "unexpected number " << primes[i] << endl;
			}
			cout << "start is " << start << " in thread " <<this_thread::get_id() << std::endl;
		};
		std::vector<std::future<void>> futures;
		auto async_policy = std::launch::deferred ;
		if(vm.count("parallel"))
			async_policy = std::launch::async;
		for(size_t i=0; i<thread_count; i++) {
			size_t start = i*thread_load;
			futures.emplace_back(std::async(async_policy, check_prime_func, primes, start));
		}
		check_prime_func(primes, thread_load*thread_count);
		for(auto& fut: futures)
			fut.get();
//		for(size_t i=0; i<primes.size(); i++) {
//			if(is_prime(primes[i])==false)
//				cout << "unexpected number " << primes[i] << endl;
//		}
	}
	std::cout << "Compiler: " << BOOST_COMPILER << endl
	              << "Platform: " << BOOST_PLATFORM << endl
	              << "Library: " << BOOST_STDLIB << endl
				  << "Boost " << BOOST_LIB_VERSION << endl;
	return 0;
}
/*
Building file: ../src/prime.cpp
Invoking: Cygwin C++ Compiler
g++ -std=c++1y -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/prime.d" -MT"src/prime.o" -o "src/prime.o" "../src/prime.cpp"
Finished building: ../src/prime.cpp

Building target: prime.exe
Invoking: Cygwin C++ Linker
g++  -o "prime.exe"  ./src/prime.o   -lboost_program_options.dll -lboost_timer
Finished building target: prime.exe

$ ./prime.exe --maxnum 9000000 --verify
!!!sieve_of_sundaram!!!
 1.006658s wall, 0.982000s user + 0.031000s system = 1.013000s CPU (100.6%)
there are 601296 prime numbers up to 9000000
start is 601296 in thread 0x20000038
start is 0 in thread 0x20000038
start is 300648 in thread 0x20000038
 4.639977s wall, 4.571000s user + 0.000000s system = 4.571000s CPU (98.5%)
Compiler: Clang version 3.9.1 (tags/RELEASE_391/final)
Platform: Cygwin
Library: GNU libstdc++ version 20160603
Boost 1_60

$ ./prime.exe --maxnum 9000000 --verify --parallel
!!!sieve_of_sundaram!!!
 1.013218s wall, 0.951000s user + 0.047000s system = 0.998000s CPU (98.5%)
there are 601296 prime numbers up to 9000000
start is 601296 in thread 0x20000038
start is 0 in thread 0x2004d568
start is 300648 in thread 0x2004d738
 3.152786s wall, 4.665000s user + 0.016000s system = 4.681000s CPU (148.5%)
Compiler: Clang version 3.9.1 (tags/RELEASE_391/final)
Platform: Cygwin
Library: GNU libstdc++ version 20160603
Boost 1_60


*/
