#include <algorithm>
#include <limits>
#include <string>
  
#include  "stdint.h"  // <--- to prevent uint64_t errors! 
 
#include "Pipes.hh"
#include "TemplateFactory.hh"
#include "StringUtils.hh"
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
 
class WordCountMapper : public HadoopPipes::Mapper {
public:
  // constructor: does nothing
  WordCountMapper( HadoopPipes::TaskContext& context ) {
  }

  // map function: receives a line, outputs (word,"1")
  // to reducer.
  void map( HadoopPipes::MapContext& context ) {
    //--- get line of text ---
    std::string line = context.getInputValue();

    //--- split it into words ---
    std::vector< std::string > words =
      HadoopUtils::splitString( line, " " );

    //--- emit each word tuple (word, "1" ) ---
    for ( unsigned int i=0; i < words.size(); i++ ) {
      context.emit( words[i], HadoopUtils::toString( 1 ) );
    }
  }
};
 
class WordCountReducer : public HadoopPipes::Reducer {
public:
  // constructor: does nothing
  WordCountReducer(HadoopPipes::TaskContext& context) {
  }

  // reduce function
  void reduce( HadoopPipes::ReduceContext& context ) {
    int count = 0;

    //--- get all tuples with the same key, and count their numbers ---
    while ( context.nextValue() ) {
      count += HadoopUtils::toInt( context.getInputValue() );
    }

    //--- emit (word, count) ---
    context.emit(context.getInputKey(), HadoopUtils::toString( count ));
  }
};
 
int main(int argc, char *argv[]) {
    std::cout << get_self_path() << " pid " << getpid() 
    << " Built with g++ "<< __GNUC__<<"." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << std::endl;
                auto t1 = std::chrono::high_resolution_clock::now();
  int ret = HadoopPipes::runTask(HadoopPipes::TemplateFactory< 
                              WordCountMapper, 
                              WordCountReducer >() );
  auto t2 = std::chrono::high_resolution_clock::now();
  // integral duration: requires duration_cast
  auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
  // fractional duration: no duration_cast needed
  std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
  std::cout << get_self_path() << " pid " << getpid() << " took " << fp_ms.count() << " ms, or " << int_ms.count() << " whole milliseconds\n";
  return ret;
}

// program output is found in /home/onega/bin/hadoop-2.7.3/logs/userlogs/application_1485663468626_0002/container_1485663468626_0002_01_000004/stdout
