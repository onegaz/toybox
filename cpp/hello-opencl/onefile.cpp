#include <iostream>
#include <vector>
#include <math.h>
#include <chrono>
#include <thread>
#include <algorithm>
#include <boost/version.hpp>
#include <boost/config.hpp>
#include <CL/cl.hpp>
//#include <OpenCL/cl.hpp>
using namespace std;

const char* mynotes=R"myopenclnotes(
download https://www.khronos.org/registry/OpenCL/api/2.1/cl.hpp to ~/oss/OpenCL/cl.hpp

$ g++ -std=c++11 -I ~/oss -O2 onefile.cpp  -framework OpenCL && ./a.out 
cpu spent 3608415 microseconds processing 131072000 values using 8 threads.
Using platform: Apple CL_PLATFORM_VERSION OpenCL 1.2 (Apr  4 2017 19:07:42)
Using device: Intel(R) HD Graphics 530
CL_DEVICE_VERSION: OpenCL 1.2 
  global memory size: 1536 MB
  local memory size: 64 KB
 Max Allocateable Memory: 384 MB
  constant memory size: 64 KB
Intel(R) HD Graphics 530 spent 1242463 microseconds processing 131072000 values using 128 bytes local memory.
 first 16 result: 
2 1 1 1 2 2 2 1 1 2 2 1 1 2 2 2 1 
gpu results match cpu results
Build with Compiler: Clang version 8.1.0 (clang-802.0.42)
Platform: Mac OS
Library: libc++ version 3700

$ otool -L a.out 
a.out:
    /System/Library/Frameworks/OpenCL.framework/Versions/A/OpenCL (compatibility version 1.0.0, current version 1.0.0)
    /usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 307.5.0)
    /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1238.60.2)

Ubuntu: 
sudo wget -O /usr/include/CL/cl.hpp https://www.khronos.org/registry/OpenCL/api/2.1/cl.hpp 
g++ -std=c++11 -pthread onefile.cpp -lOpenCL
)myopenclnotes";

const size_t data_num = 1024*1024*125; // 125 OK, 126 Abort trap: 6
int A[data_num], B[data_num], C[data_num], cpu_results[data_num];

// OpenCL kernel function
string kernel_code= R"(
void kernel gpu_math(global const int* A, global const int* B, global int* C){
    __local float data, result, result2;
    __local int i;
    i = get_global_id(0);
    data = A[i]; 
    result = atan( exp(atan(asin(1.0f/(1.0f+exp(data))))) );
    data = B[i];
    result2 = exp(atan(asin(1.0f/(1.0f+exp(data)))));
    result = result + result2;
    C[i]=convert_int_rtz(result);
}
)";

inline void cpu_math(int* A, int* B, int* C)
{
    float data, result, result2;
    data = *A; 
    result = atan( exp(atan(asin(1.0f/(1.0f+exp( data))))) );
    data = *B;
    result2 = exp(atan(asin(1.0f/(1.0f+exp(data)))));
    result = result + result2;
    *C = (int)(result);
}

void cpu_task(int* A, int* B, int* C, size_t start_pos, size_t len)
{
    for (size_t i=0; i<len; i++)
        cpu_math (A+i+start_pos, B+i+start_pos, C+i+start_pos);
}

unsigned long cpu_processor()
{
    unsigned long const min_per_thread=65536;
    unsigned long const max_threads= (data_num+min_per_thread-1)/min_per_thread; 
    unsigned long hw_threads = thread::hardware_concurrency();
    unsigned long const num_threads= std::min(hw_threads!=0?hw_threads:2,max_threads);
    unsigned long const block_size=data_num/num_threads;
    vector<thread> threads(num_threads-1);
    for(unsigned long i=0;i<(num_threads-1);++i)
         threads[i]=std::thread(cpu_task, A, B, cpu_results, i*block_size, block_size);
    cpu_task(A, B, cpu_results, block_size *(num_threads-1), data_num-block_size *(num_threads-1));
    for_each(threads.begin(), threads.end(), std::mem_fn(&thread::join)); 
    return num_threads;
}

void gpu_processor(cl::Device& default_device, size_t local_mem_size)
{
    cl::Context context({default_device});
    cl::Program::Sources sources;
    sources.push_back({kernel_code.c_str(), kernel_code.length()});
    cl::Program program(context,sources);
    if(program.build({default_device})!=CL_SUCCESS){
        cout<<" Error building: "<<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device)<<"\n";
        exit(1);
    }
 
    // create buffers on the device
    cl::Buffer buffer_A(context,CL_MEM_READ_WRITE,sizeof(int)*data_num);
    cl::Buffer buffer_B(context,CL_MEM_READ_WRITE,sizeof(int)*data_num);
    cl::Buffer buffer_C(context,CL_MEM_READ_WRITE,sizeof(int)*data_num);
    //create queue to which we will push commands for the device.
    cl::CommandQueue queue(context,default_device);
 
    //write arrays A and B to the device
    queue.enqueueWriteBuffer(buffer_A,CL_TRUE,0,sizeof(int)*data_num,A);
    queue.enqueueWriteBuffer(buffer_B,CL_TRUE,0,sizeof(int)*data_num,B);
    cl::NDRange gpu_local_size(local_mem_size); //cl::NullRange and 256 OK, 512 not work
    //alternative way to run the kernel
#if 0
    cl::Kernel kernel_proc=cl::Kernel(program,"gpu_math");
    kernel_proc.setArg(0, buffer_A);
    kernel_proc.setArg(1, buffer_B);
    kernel_proc.setArg(2, buffer_C);
    queue.enqueueNDRangeKernel(kernel_proc, cl::NullRange, cl::NDRange(data_num), gpu_local_size);
    queue.finish();
#else
    //cl 1.2
    cl::make_kernel<cl::Buffer, cl::Buffer, cl::Buffer> kernel_proc(cl::Kernel(program,"gpu_math"));
    cl::EnqueueArgs eargs(queue, cl::NullRange, cl::NDRange(data_num), gpu_local_size);
    kernel_proc(eargs, buffer_A, buffer_B, buffer_C).wait();
#endif
    //read result from the device to host
    queue.enqueueReadBuffer(buffer_C, CL_TRUE, 0, sizeof(int)*data_num, C);
    queue.finish();    
}

int main(int argc, char**){
    int i = 0;
    for(int& data: A)
        data = (++i%7);
    for (int& data: B)
        data = (++i%5);
    
    unsigned long num_threads = 1;
    if (0)
    {
        auto start_time = chrono::high_resolution_clock::now();
        cpu_task(A, B, cpu_results, 0, data_num);
        auto end_time = chrono::high_resolution_clock::now();
        cout <<"cpu spent " << chrono::duration_cast<chrono::microseconds>(end_time - start_time).count()
            << " microseconds processing " << data_num << " values using " << num_threads << " threads.\n";        
    }
    auto start_time = chrono::high_resolution_clock::now();
    num_threads = cpu_processor();
    auto end_time = chrono::high_resolution_clock::now();
    cout <<"cpu spent " << chrono::duration_cast<chrono::microseconds>(end_time - start_time).count()
        << " microseconds processing " << data_num << " values using " << num_threads << " threads.\n";
    
    //get all platforms (drivers)
    vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);
    if(all_platforms.size()==0){
        cout<<" No platforms found. Check OpenCL installation!\n";
        return 1;
    }
    cl::Platform default_platform=all_platforms[0];
    cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>()
        << " CL_PLATFORM_VERSION " << default_platform.getInfo<CL_PLATFORM_VERSION>()
        <<"\n";
    //get default device of the default platform
    std::vector<cl::Device> all_devices;
    default_platform.getDevices(CL_DEVICE_TYPE_GPU , &all_devices);
    if(all_devices.size()==0){
        cout<<" No GPU devices found. Check OpenCL installation!\n";
        return 1;
    }
    cl::Device default_device=all_devices[0];
    cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() 
         << "\nCL_DEVICE_VERSION: " << default_device.getInfo<CL_DEVICE_VERSION>()
         << "\n  global memory size: "
         << default_device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / 1024 / 1024 << " MB\n" 
         << "  local memory size: "
         << default_device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() / 1024 << " KB\n" 
         << " Max Allocateable Memory: " << default_device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() / 1024 / 1024
         << " MB\n  constant memory size: "
         << default_device.getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>() / 1024 << " KB\n";
    size_t local_mem_size = 128;
    start_time = chrono::high_resolution_clock::now();
    gpu_processor(default_device, local_mem_size);
    end_time = chrono::high_resolution_clock::now();
    cout << default_device.getInfo<CL_DEVICE_NAME>() << " spent " 
        << chrono::duration_cast<chrono::microseconds>(end_time - start_time).count()
        << " microseconds processing " << data_num << " values using " 
        << local_mem_size //(size_t*)(gpu_local_size)[0]
        << " bytes local memory.\n";
    cout << " first 16 result: \n";
    i = 0;
    for(int result: C){
        std::cout<<result<<" ";
        if (++i>16)
            break;
    }
    cout << endl;
    
    int mismatch_num = 0;
    for ( i=0; i<data_num; i++)
    {
        if ( abs(C[i]-cpu_results[i])>0)
        {
            cout << "gpu result: " << C[i] <<", cpu result: " << cpu_results[i] 
                 << " at position " << i << endl;
            ++mismatch_num;
            if (mismatch_num>10)
                break;
        }
    }
    if (mismatch_num < 1)
        cout << "gpu results match cpu results\n";
    cout << "Build with Compiler: " << BOOST_COMPILER << std::endl
          << "Platform: " << BOOST_PLATFORM << std::endl
          << "Library: " << BOOST_STDLIB << std::endl
          ;
    return 0;
}
