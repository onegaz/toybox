#include "hdfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <memory>
#include <array>
#include <fstream>
#include <type_traits>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/limits.h>
#define gettid() syscall(SYS_gettid)

const char* get_self_path()
{
    static char selfpath[PATH_MAX];
    if (selfpath[0])
        return selfpath;
    char path[PATH_MAX];
    struct stat info;
    pid_t pid = getpid();
    sprintf(path, "/proc/%d/exe", pid);
    if (readlink(path, selfpath, PATH_MAX) == -1)
        perror("readlink");
    return selfpath;
}

void write_hdfs(hdfsFS fs, const std::string& file_path)
{
    int flags = O_WRONLY | O_CREAT | O_APPEND;
    auto t1 = std::chrono::high_resolution_clock::now();
    hdfsFile writeFile = hdfsOpenFile(fs, file_path.c_str(), flags, 0, 0, 0);

    if (!writeFile)
    {
        fprintf(stderr, "Failed to open %s for writing!\n", file_path.c_str());
        exit(-1);
    }

    auto in_time_t = std::chrono::system_clock::to_time_t(t1);
    std::stringstream ss;

    ss << std::put_time(std::localtime(&in_time_t),
                        "%Y-%m-%d %X"); // need g++ 5.x and above
    ss << " first C++ client for HDFS." << std::endl;
    ss << get_self_path() << " pid " << getpid() << " Built with g++ " << __GNUC__ << "."
       << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << std::endl;
    ;
    std::string message = ss.str();
    tSize num_written_bytes =
        hdfsWrite(fs, writeFile, (void*) message.c_str(), message.length() + 1);
    if (hdfsFlush(fs, writeFile))
    {
        fprintf(stderr, "Failed to 'flush' %s\n", file_path.c_str());
        exit(-1);
    }
    hdfsCloseFile(fs, writeFile);

    auto t2 = std::chrono::high_resolution_clock::now();
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    std::cout << "it took " << int_ms
              << " milliseconds to hdfsOpenFile/hdfsWrite/hdfsFlush/hdfsCloseFile\n";
}

std::string exec(const char* cmd)
{
    std::array<char, 4096> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
        throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get()))
    {
        if (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

void set_classpath()
{
	// CLASSPATH=$(JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64 ~/bin/hadoop-3.1.0/bin/hadoop classpath --glob)
	std::stringstream strm;
	strm << getenv("HADOOP_HOME") << "/bin/hadoop classpath --glob";
	std::string classpath = exec(strm.str().c_str());
//	std::cout << strm.str() << std::endl << classpath << std::endl;
	setenv("CLASSPATH", classpath.c_str(), 1);
}

int main(int argc, char** argv)
{
    std::cout << "This program will create /tmp if it does not exist in HDFS, or you can "
                 "create ahead of time: bin/hdfs dfs -mkdir /tmp"
              << std::endl;

    if (!getenv("HADOOP_HOME"))
    {
        std::cerr << "Please set environment variable HADOOP_HOME\n";
        return -1;
    }
    else
    {
        std::stringstream strm;
        strm << getenv("HADOOP_HOME") << "/bin/hadoop";
        std::ifstream testfile(strm.str().c_str());
        if (!testfile)
        {
            std::cerr << "Can't open " << strm.str()
                      << ", please check environment variable HADOOP_HOME "
                      << getenv("HADOOP_HOME") << std::endl;
            return -1;
        }
    }

    if (!getenv("JAVA_HOME"))
    {
        std::cerr << "Please set environment variable JAVA_HOME\n";
        return -1;
    }
    else
    {
        std::stringstream strm;
        strm << getenv("JAVA_HOME") << "/jre/lib/amd64/server/libjvm.so";
        std::ifstream testfile(strm.str().c_str());
        if (!testfile)
        {
            std::cerr << "Can't open " << strm.str()
                      << ", please check environment variable JAVA_HOME "
                      << getenv("JAVA_HOME") << std::endl;
            return -1;
        }
    }

    set_classpath();

    hdfsFS fs = hdfsConnect("hdfs://localhost", 0);
    std::unique_ptr<std::remove_pointer<hdfsFS>::type, decltype(&hdfsDisconnect)> fs_inst{
        fs, hdfsDisconnect};

    if (!fs)
    {
        std::cout
            << "Failed to connect to HDFS, please start HDFS first: sbin/start-dfs.sh"
            << std::endl;
        std::cout << "Make sure fs.defaultFS is defined in core-site.xml" << std::endl;
        return -1;
    }

    const char* workingdir = "/tmp";
    int ret = hdfsExists(fs, workingdir);
    if (ret)
    {
        std::cout << workingdir << " does not exist, try to create it now" << std::endl;
        ret = hdfsCreateDirectory(fs, workingdir);
        if (ret)
        {
            std::cout << "failed to create directory " << workingdir << " return " << ret
                      << std::endl;
            return ret;
        }
        std::cout << workingdir
                  << " created. You can delete it by command: bin/hdfs dfs -rm -r "
                  << workingdir << std::endl;
    }

    char writePath[PATH_MAX];
    sprintf(writePath, "%s/testfile.txt", workingdir);

    for (int i = 0; i < 5; ++i)
        write_hdfs(fs, writePath);

    std::cout << "Check content with command: bin/hdfs dfs -cat " << writePath
              << std::endl;
}

// [onega@localhost hellohdfs]$ CLASSPATH=$(JAVA_HOME=/usr/lib/jvm/java ~/bin/hadoop-2.7.3/bin/hadoop classpath) LD_LIBRARY_PATH=/home/onega/bin/hadoop-2.7.3/lib/native:/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.60-2.b27.el7_1.x86_64/jre/lib/amd64/server ./hellohdfs
// loadFileSystems error:
// (unable to get stack trace for java.lang.NoClassDefFoundError exception: ExceptionUtils::getStackTrace error.)
// hdfsBuilderConnect(forceNewInstance=0, nn=default, port=0, kerbTicketCachePath=(NULL), userName=(NULL)) error:
// (unable to get stack trace for java.lang.NoClassDefFoundError exception: ExceptionUtils::getStackTrace error.)
// hdfsOpenFile(/tmp/testfile.txt): constructNewObjectOfPath error:
// (unable to get stack trace for java.lang.NoClassDefFoundError exception: ExceptionUtils::getStackTrace error.)
// Failed to open /tmp/testfile.txt for writing!
//
// [onega@localhost hellohdfs]$ CLASSPATH=$(JAVA_HOME=/usr/lib/jvm/java ~/bin/hadoop-2.7.3/bin/hadoop classpath --glob) LD_LIBRARY_PATH=/home/onega/bin/hadoop-2.7.3/lib/native:/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.60-2.b27.el7_1.x86_64/jre/lib/amd64/server ./hellohdfs
// [onega@localhost hellohdfs]$

// [onega@localhost hellohdfs]$ CLASSPATH=$(JAVA_HOME=/usr/lib/jvm/java ~/bin/hadoop-2.7.3/bin/hadoop classpath --glob) LD_LIBRARY_PATH=/home/onega/bin/hadoop-2.7.3/lib/native:/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.60-2.b27.el7_1.x86_64/jre/lib/amd64/server:/home/onega/gcc-6.3.0/lib64 ./hellohdfs
// [onega@localhost hellohdfs]$
