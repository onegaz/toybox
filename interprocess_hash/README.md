An experiment C++ program that checks performance of hashmap in shared memory vs local memory.
$ ./interprocess_hash.exe --remove && ./interprocess_hash.exe && ./interprocess_hash.exe --dump
./interprocess_hash access hashmap 8096000 times.
Built with g++ 5.4.0
HashMap in shared memory took 2127.77 ms, or 2127 whole milliseconds
HashMap in local memory took 892.613 ms, or 892 whole milliseconds
89377 value is 1000
5125 value is 1000
77668 value is 1000
112764 value is 1000
...