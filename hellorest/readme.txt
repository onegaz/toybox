start pistache/examples/hello_server first
[onega@localhost rest]$ make && ./run.sh
g++ -o cpprestclient1 -std=c++11 -I /home/onega/src/cpprestsdk/Release/include -pthread -g cpprestclient1.cpp -L /home/onega/src/cpprestsdk/Release/Binaries -L
 /home/onega/gcc-6.3.0/lib64 -lcpprest -lcommon_utilities -lboost_system -lssl -lcrypto -ldl -lstdc++
echo LD_LIBRARY_PATH=/home/onega/gcc-6.3.0/lib64:/home/onega/src/cpprestsdk/Release/Binaries:'$LD_LIBRARY_PATH' ./cpprestclient1
LD_LIBRARY_PATH=/home/onega/gcc-6.3.0/lib64:/home/onega/src/cpprestsdk/Release/Binaries:$LD_LIBRARY_PATH ./cpprestclient1
test_read_to_end start
Content Length: 33
operator() read 33 characters, {"result":"Operate successfully"}
extract_string done
extract_string return 33
        (10 segments)
        (4 segments)
/home/onega/src/cpprestsdk/Release/Binaries/libcpprest.so.2.9   (8 segments)
/home/onega/src/cpprestsdk/Release/Binaries/libcommon_utilities.so      (7 segments)
/usr/local/lib/libboost_system.so.1.59.0        (7 segments)
/lib64/libssl.so.10     (7 segments)
/lib64/libcrypto.so.10  (7 segments)
/lib64/libdl.so.2       (9 segments)
/home/onega/gcc-6.3.0/lib64/libstdc++.so.6      (7 segments)
/lib64/libm.so.6        (9 segments)
/home/onega/gcc-6.3.0/lib64/libgcc_s.so.1       (6 segments)
/lib64/libpthread.so.0  (9 segments)
/lib64/libc.so.6        (10 segments)
/usr/local/lib/libboost_thread.so.1.59.0        (7 segments)
/usr/local/lib/libboost_atomic.so.1.59.0        (7 segments)
/usr/local/lib/libboost_chrono.so.1.59.0        (7 segments)
/usr/local/lib/libboost_random.so.1.59.0        (7 segments)
/usr/local/lib/libboost_regex.so.1.59.0 (7 segments)
/usr/local/lib/libboost_date_time.so.1.59.0     (7 segments)
/usr/local/lib/libboost_filesystem.so.1.59.0    (7 segments)
/lib64/libz.so.1        (7 segments)
/lib64/ld-linux-x86-64.so.2     (7 segments)
/home/onega/src/cpprestsdk/Release/Binaries/libunittestpp.so    (7 segments)
/lib64/librt.so.1       (9 segments)
/lib64/libgssapi_krb5.so.2      (7 segments)
/lib64/libkrb5.so.3     (7 segments)
/lib64/libcom_err.so.2  (8 segments)
/lib64/libk5crypto.so.3 (7 segments)
/lib64/libicudata.so.50 (6 segments)
/lib64/libicui18n.so.50 (7 segments)
/lib64/libicuuc.so.50   (7 segments)
/lib64/libkrb5support.so.0      (7 segments)
/lib64/libkeyutils.so.1 (7 segments)
/lib64/libresolv.so.2   (9 segments)
/lib64/libselinux.so.1  (8 segments)
/lib64/libpcre.so.1     (7 segments)
/lib64/liblzma.so.5     (7 segments)
Built with g++ 4.8.5./cpprestclient1 pid 6345 done

