#include <iostream>
#include <chrono>
#include <thread>

int main(int argc, char* argv[]) {
    std::thread::id this_id = std::this_thread::get_id();
    using namespace std::chrono;
    std::cout << argv[0] << " thread id " << this_id << " prints a message every 20ms" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<800; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(40) );
        std::cout << argv[0] << " thread id " << this_id << " prints a message every 20ms, i=" << i << std::endl;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end-start;
    std::cout << argv[0] << " thread id " << this_id <<" Waited " << elapsed.count() << " ms\n";
    return 0;
}
// g++ -o mytool.exe -std=c++0x mytool.cpp
