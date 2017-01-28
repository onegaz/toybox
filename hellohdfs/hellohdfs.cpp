#include "hdfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/limits.h>
#define gettid() syscall(SYS_gettid)

const char* get_self_path() {
  static char selfpath[PATH_MAX];
  if(selfpath[0])
    return selfpath;
  char path[PATH_MAX];
  struct stat info;
  pid_t pid = getpid();
  sprintf(path, "/proc/%d/exe", pid);
  if (readlink(path, selfpath, PATH_MAX) == -1)
    perror("readlink");
    return selfpath;
}

int main(int argc, char **argv) {
    std::cout << "This program will create /tmp if it does not exist in HDFS, or you can create ahead of time: bin/hdfs dfs -mkdir /tmp" << std::endl;
    hdfsFS fs = hdfsConnect("default", 0);
    if (fs == NULL) {
      std::cout << "Failed to connect to HDFS, please start HDFS first: sbin/start-dfs.sh" << std::endl;
      std::cout << "Make sure fs.defaultFS is defined in core-site.xml" << std::endl;
      return -1;
    }
    int ret = 0;
    const char* workingdir = "/tmp";
    ret = hdfsExists(fs, workingdir);
    if(ret) {
      std::cout << workingdir << " does not exist, try to create it now" << std::endl;
      ret = hdfsCreateDirectory(fs, workingdir);
      if (ret) {
        std::cout << "failed to create directory " << workingdir << " return " << ret << std::endl;
        return ret;
      }
      std::cout << workingdir << " created. You can delete it by command: bin/hdfs dfs -rm -r " << workingdir << std::endl;
    }
    char writePath[PATH_MAX];
    sprintf(writePath, "%s/testfile.txt", workingdir);
    ret = hdfsExists(fs, writePath);
    int flags = O_WRONLY |O_CREAT;
    if (ret==0) // file exists
      flags |= O_APPEND;
    hdfsFile writeFile = hdfsOpenFile(fs, writePath, flags, 0, 0, 0);
    if(!writeFile) {
          fprintf(stderr, "Failed to open %s for writing!\n", writePath);
          exit(-1);
    }
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X"); // need g++ 5.x and above
    ss << " first C++ client for HDFS."<< std::endl;
    ss << get_self_path() << " pid " << getpid() << " Built with g++ "<< __GNUC__<<"." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << std::endl;;
    std::string message = ss.str();
    tSize num_written_bytes = hdfsWrite(fs, writeFile, (void*)message.c_str(), message.length()+1);
    if (hdfsFlush(fs, writeFile)) {
           fprintf(stderr, "Failed to 'flush' %s\n", writePath);
          exit(-1);
    }
    hdfsCloseFile(fs, writeFile);
    std::cout << "Check content with command: bin/hdfs dfs -cat /tmp/testfile.txt" << std::endl;
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
