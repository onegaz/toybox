#include <stdio.h>
#include <string>
#include <iostream>
#include <map>
#include <iostream>
#include <iterator>
#include <syslog.h>
#include <sstream>
#include <chrono>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/limits.h>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sys/stat.h>
#include <unordered_map>
#define gettid() syscall(SYS_gettid)
using namespace std;
 
int mapper(){
    string key;
    while(cin>>key){
        cout<<key<<"\t"<<1<<endl;
    }
    return 0;
}

int reducer(){
    string key;
    int value = 0;
    unordered_map<string, int> word2count;
    while(cin>>key){
        cin>>value;
        auto it = word2count.find(key);
        if(it != word2count.end()){
            (it->second)+= value;
        }
        else{
            word2count.insert(make_pair(key, value));
        }
    }
    for(const auto &it:word2count){
        cout<<it.first<<"\t"<<it.second<<endl;
    }
    return 0;
}

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

int main(int argc, char* argv[]) {
    int ret = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    if(strcasestr(get_self_path(), "mapper")) {
        ret = mapper();
    } else {
        ret = reducer();
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    // integral duration: requires duration_cast
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    // fractional duration: no duration_cast needed
    std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
    std::stringstream ss;
    ss << get_self_path() << " pid " << getpid() << " started by user " << getuid()
    << " Built with g++ "<< __GNUC__<<"." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__;
    ss << " took " << int_ms.count() << " milliseconds";
    setlogmask (LOG_UPTO (LOG_DEBUG));
    openlog ("hadoop-streaming", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog (LOG_INFO, ss.str().c_str());
    closelog ();
    return ret;
}
// for CentOS syslog, check /etc/rsyslog.conf 