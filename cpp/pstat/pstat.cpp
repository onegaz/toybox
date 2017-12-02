#include <unistd.h>
#include <algorithm>
#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <chrono>
#include <boost/config.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <boost/version.hpp>

int pid = 19922;
std::vector<std::string> with_fields;
bool show_long_line = false;

/*
g++ -o pstat -g -std=c++11 pstat.cpp -lboost_filesystem -lboost_system -lboost_program_options
g++ -o pstat -g -O2 -std=c++11 pstat.cpp -lboost_filesystem -lboost_system -lboost_program_options

$ ./pstat -p 2638 -w utime pid
/proc/2638/task/2638/stat
pid			2638
utime			0
/proc/2638/task/2643/stat
pid			2643
utime			1115
/proc/2638/task/2650/stat
pid			2650
utime			25438
/proc/2638/task/2662/stat
pid			2662
utime			1311
/proc/2638/task/2663/stat
pid			2663
utime			1286
/proc/2638/task/2664/stat
pid			2664
utime			1320
/proc/2638/task/2665/stat
pid			2665
utime			45
/proc/2638/task/2666/stat
pid			2666
utime			1686
/proc/2638/task/2667/stat
pid			2667
utime			19
/proc/2638/task/2668/stat
pid			2668
utime			3
/proc/2638/task/2669/stat
pid			2669
utime			0
/proc/2638/task/2670/stat
pid			2670
utime			0
/proc/2638/task/2671/stat
pid			2671
utime			9946
/proc/2638/task/2672/stat
pid			2672
utime			9942
/proc/2638/task/2673/stat
pid			2673
utime			8667
/proc/2638/task/2674/stat
pid			2674
utime			4067
/proc/2638/task/6125/stat
pid			6125
utime			6
./pstat pid 6304 used 46 milliseconds, sysconf (_SC_CLK_TCK) return 100

$ sudo cat /proc/2638/task/14022/stack
[<ffffffff89d0e544>] futex_wait_queue_me+0xc4/0x120
[<ffffffff89d0f1c6>] futex_wait+0x116/0x270
[<ffffffff89d116b4>] do_futex+0x214/0x540
[<ffffffff89d11a61>] SyS_futex+0x81/0x180
[<ffffffff8a4d66bb>] entry_SYSCALL_64_fastpath+0x1e/0xad
[<ffffffffffffffff>] 0xffffffffffffffff

$ find /proc/19922/task -name stat -type f
/proc/19922/task/19922/stat
/proc/19922/task/19925/stat
/proc/19922/task/19926/stat
/proc/19922/task/19927/stat
$ cat /proc/19922/task/19922/stat
19922 (chrome) S 3888 2249 2249 0 -1 4194304 259024 0 23 0 1049 779 0 0 20 0 4 0 1025530
536297472 16518 18446744073709551615 94532398252032 94532511898816 140723068890688 0 0 0 0
4098 1073741824 1 0 0 17 3 0 0 18 0 0 94532511899648 94532517920368 94532551012352
140723068896610 140723068897138 140723068897138 140723068899305 0
*/

const char* man_proc = R"from_man(       /proc/[pid]/stat
              Status information about the process.  This is used by ps(1).  It is defined in the kernel source file fs/proc/array.c.

              The fields, in order, with their proper scanf(3) format specifiers, are:

              (1) pid  %d
                        The process ID.

              (2) comm  %s
                        The filename of the executable, in parentheses.  This is visible whether or not the executable is swapped out.

              (3) state  %c
                        One of the following characters, indicating process state:

                        R  Running

                        S  Sleeping in an interruptible wait

                        D  Waiting in uninterruptible disk sleep

                        Z  Zombie

                        T  Stopped (on a signal) or (before Linux 2.6.33) trace stopped

                        t  Tracing stop (Linux 2.6.33 onward)

                        W  Paging (only before Linux 2.6.0)

                        X  Dead (from Linux 2.6.0 onward)

                        x  Dead (Linux 2.6.33 to 3.13 only)

                        K  Wakekill (Linux 2.6.33 to 3.13 only)

                        W  Waking (Linux 2.6.33 to 3.13 only)

                        P  Parked (Linux 3.9 to 3.13 only)

              (4) ppid  %d
                        The PID of the parent of this process.

              (5) pgrp  %d
                        The process group ID of the process.

              (6) session  %d
                        The session ID of the process.

              (7) tty_nr  %d
                        The controlling terminal of the process.  (The minor device number is contained in the combination of bits 31 to 20 and 7 to 0; the major device number is in bits 15 to 8.)

              (8) tpgid  %d
                        The ID of the foreground process group of the controlling terminal of the process.

              (9) flags  %u
                        The kernel flags word of the process.  For bit meanings, see the PF_* defines in the Linux kernel source file include/linux/sched.h.  Details depend on the kernel version.

                        The format for this field was %lu before Linux 2.6.

              (10) minflt  %lu
                        The number of minor faults the process has made which have not required loading a memory page from disk.

              (11) cminflt  %lu
                        The number of minor faults that the process's waited-for children have made.

              (12) majflt  %lu
                        The number of major faults the process has made which have required loading a memory page from disk.

              (13) cmajflt  %lu
                        The number of major faults that the process's waited-for children have made.

              (14) utime  %lu
                        Amount of time that this process has been scheduled in user mode, measured in clock ticks (divide by sysconf(_SC_CLK_TCK)).  This includes guest time, guest_time (time spent running a virtual
                        CPU, see below), so that applications that are not aware of the guest time field do not lose that time from their calculations.

              (15) stime  %lu
                        Amount of time that this process has been scheduled in kernel mode, measured in clock ticks (divide by sysconf(_SC_CLK_TCK)).

              (16) cutime  %ld
                        Amount  of  time that this process's waited-for children have been scheduled in user mode, measured in clock ticks (divide by sysconf(_SC_CLK_TCK)).  (See also times(2).)  This includes guest
                        time, cguest_time (time spent running a virtual CPU, see below).

              (17) cstime  %ld
                        Amount of time that this process's waited-for children have been scheduled in kernel mode, measured in clock ticks (divide by sysconf(_SC_CLK_TCK)).

              (18) priority  %ld
                        (Explanation for Linux 2.6) For processes running a real-time scheduling policy (policy below; see sched_setscheduler(2)), this is the negated scheduling priority, minus one; that is, a  num‐
                        ber in the range -2 to -100, corresponding to real-time priorities 1 to 99.  For processes running under a non-real-time scheduling policy, this is the raw nice value (setpriority(2)) as rep‐
                        resented in the kernel.  The kernel stores nice values as numbers in the range 0 (high) to 39 (low), corresponding to the user-visible nice range of -20 to 19.

                        Before Linux 2.6, this was a scaled value based on the scheduler weighting given to this process.

              (19) nice  %ld
                        The nice value (see setpriority(2)), a value in the range 19 (low priority) to -20 (high priority).

              (20) num_threads  %ld
                        Number of threads in this process (since Linux 2.6).  Before kernel 2.6, this field was hard coded to 0 as a placeholder for an earlier removed field.

              (21) itrealvalue  %ld
                        The time in jiffies before the next SIGALRM is sent to the process due to an interval timer.  Since kernel 2.6.17, this field is no longer maintained, and is hard coded as 0.

              (22) starttime  %llu
                        The time the process started after system boot.  In kernels before Linux 2.6, this value was expressed in jiffies.  Since Linux  2.6,  the  value  is  expressed  in  clock  ticks  (divide  by
                        sysconf(_SC_CLK_TCK)).

                        The format for this field was %lu before Linux 2.6.

              (23) vsize  %lu
                        Virtual memory size in bytes.

              (24) rss  %ld
                        Resident  Set  Size:  number  of  pages the process has in real memory.  This is just the pages which count toward text, data, or stack space.  This does not include pages which have not been
                        demand-loaded in, or which are swapped out.

              (25) rsslim  %lu
                        Current soft limit in bytes on the rss of the process; see the description of RLIMIT_RSS in getrlimit(2).

              (26) startcode  %lu
                        The address above which program text can run.

              (27) endcode  %lu
                        The address below which program text can run.

              (28) startstack  %lu
                        The address of the start (i.e., bottom) of the stack.

              (29) kstkesp  %lu
                        The current value of ESP (stack pointer), as found in the kernel stack page for the process.

              (30) kstkeip  %lu
                        The current EIP (instruction pointer).

              (31) signal  %lu
                        The bitmap of pending signals, displayed as a decimal number.  Obsolete, because it does not provide information on real-time signals; use /proc/[pid]/status instead.

              (32) blocked  %lu
                        The bitmap of blocked signals, displayed as a decimal number.  Obsolete, because it does not provide information on real-time signals; use /proc/[pid]/status instead.

              (33) sigignore  %lu
                        The bitmap of ignored signals, displayed as a decimal number.  Obsolete, because it does not provide information on real-time signals; use /proc/[pid]/status instead.

              (34) sigcatch  %lu
                        The bitmap of caught signals, displayed as a decimal number.  Obsolete, because it does not provide information on real-time signals; use /proc/[pid]/status instead.

              (35) wchan  %lu
                        This is the "channel" in which the process is waiting.  It is the address of a location in the kernel where the process  is  sleeping.   The  corresponding  symbolic  name  can  be  found  in
                        /proc/[pid]/wchan.

              (36) nswap  %lu
                        Number of pages swapped (not maintained).

              (37) cnswap  %lu
                        Cumulative nswap for child processes (not maintained).

              (38) exit_signal  %d  (since Linux 2.1.22)
                        Signal to be sent to parent when we die.

              (39) processor  %d  (since Linux 2.2.8)
                        CPU number last executed on.

              (40) rt_priority  %u  (since Linux 2.5.19)
                        Real-time scheduling priority, a number in the range 1 to 99 for processes scheduled under a real-time policy, or 0, for non-real-time processes (see sched_setscheduler(2)).

              (41) policy  %u  (since Linux 2.5.19)
                        Scheduling policy (see sched_setscheduler(2)).  Decode using the SCHED_* constants in linux/sched.h.

                        The format for this field was %lu before Linux 2.6.22.

              (42) delayacct_blkio_ticks  %llu  (since Linux 2.6.18)
                        Aggregated block I/O delays, measured in clock ticks (centiseconds).

              (43) guest_time  %lu  (since Linux 2.6.24)
                        Guest time of the process (time spent running a virtual CPU for a guest operating system), measured in clock ticks (divide by sysconf(_SC_CLK_TCK)).

              (44) cguest_time  %ld  (since Linux 2.6.24)
                        Guest time of the process's children, measured in clock ticks (divide by sysconf(_SC_CLK_TCK)).

              (45) start_data  %lu  (since Linux 3.3)
                        Address above which program initialized and uninitialized (BSS) data are placed.

              (46) end_data  %lu  (since Linux 3.3)
                        Address below which program initialized and uninitialized (BSS) data are placed.

              (47) start_brk  %lu  (since Linux 3.3)
                        Address above which program heap can be expanded with brk(2).

              (48) arg_start  %lu  (since Linux 3.5)
                        Address above which program command-line arguments (argv) are placed.

              (49) arg_end  %lu  (since Linux 3.5)
                        Address below program command-line arguments (argv) are placed.

              (50) env_start  %lu  (since Linux 3.5)
                        Address above which program environment is placed.

              (51) env_end  %lu  (since Linux 3.5)
                        Address below which program environment is placed.

              (52) exit_code  %d  (since Linux 3.5)
                        The thread's exit status in the form reported by waitpid(2).
)from_man";

void add_options(boost::program_options::options_description& desc)
{
    desc.add_options()
    	("help,h", "Produce help message")
    	("pid,p", boost::program_options::value<decltype(pid)>(&pid),"specify pid to check")
    	("show_long_line,s", boost::program_options::value<decltype(show_long_line)>(&show_long_line),"show_long_line")
    	("with_fields,w", boost::program_options::value<std::vector<std::string> >(&with_fields)->multitoken(), "Specify which fields to display, eg: -w utime pid")
    	("stack", "Display stack, need sudo");
    	("man,m", "Display related man page");
}

std::vector<std::string> fields_name{
    "pid",         "comm",        "state",
    "ppid",        "pgrp",        "session",
    "tty_nr",      "tpgid",       "flags",
    "minflt",      "cminflt",     "majflt",
    "cmajflt",     "utime",       "stime",
    "cutime",      "cstime",      "priority",
    "nice",        "num_threads", "itrealvalue",
    "starttime",   "vsize",       "rss",
    "rsslim",      "startcode",   "endcode",
    "startstack",  "kstkesp",     "kstkeip",
    "signal",      "blocked",     "sigignore",
    "sigcatch",    "wchan",       "nswap",
    "cnswap",      "exit_signal", "processor",
    "rt_priority", "policy",      "delayacct_blkio_ticks",
    "guest_time",  "cguest_time", "start_data",
    "end_data",    "start_brk",   "arg_start",
    "arg_end",     "end_start",   "env_end",
    "exit_code"};

int display_proc_stat(const char* filename)
{
    std::string statline;
    std::ifstream statfile(filename);
    std::getline(statfile, statline);

    std::cout << filename << "\n";

    if (show_long_line)
        std::cout << statline << std::endl;

    std::string separator1("");   // don't let quoted arguments escape themselves
    std::string separator2(" ");  // split on spaces
    std::string separator3("()"); // let it have quoted arguments

    boost::escaped_list_separator<char> els(separator1, separator2, separator3);
    boost::tokenizer<boost::escaped_list_separator<char>> tok(statline, els);
    std::vector<std::string> proc_fields;

    for (boost::tokenizer<boost::escaped_list_separator<char>>::iterator beg =
             tok.begin();
         beg != tok.end(); ++beg)
    {
        proc_fields.emplace_back(*beg);
    }

    if (proc_fields.size() != fields_name.size())
    {
        std::cout << "Unknown format, please check man proc\n";
        std::cout << statline << std::endl;
        return 0;
    }

    int max_field_name_len = 0;

    for (auto& name : fields_name)
        max_field_name_len = std::max(max_field_name_len, (int) name.length());

    max_field_name_len = (max_field_name_len / 8 + 1) * 8;

    auto print_tabs = [](int n) {
        for (int i = 0; i < n; ++i)
            std::cout << "\t";
    };

    for (int i = 0; i < fields_name.size(); ++i)
    {
        if (with_fields.size() &&
            std::find(std::begin(with_fields), std::end(with_fields), fields_name[i]) ==
                std::end(with_fields))
            continue;

        if (fields_name[i] != "state")
        {
            std::cout << fields_name[i];
            print_tabs(3 - fields_name[i].length() / 8);
            std::cout << proc_fields[i] << "\n";
            continue;
        }

        std::cout << fields_name[i];
        print_tabs(3 - fields_name[i].length() / 8);
        std::cout << proc_fields[i] << "\t";
        for (auto state : proc_fields[i])
        {
            switch (state)
            {
            case 'R':
                std::cout << "Running ";
                break;
            case 'S':
                std::cout << "Sleeping in an interruptible wait ";
                break;
            case 'D':
                std::cout << "Waiting in uninterruptible disk sleep ";
                break;
            case 'Z':
                std::cout << "Zombie ";
                break;
            case 'T':
                std::cout
                    << "Stopped (on a signal) or (before Linux 2.6.33) trace stopped ";
                break;
            case 't':
                std::cout << "Tracing stop (Linux 2.6.33 onward) ";
                break;
            case 'W':
                std::cout << "Paging (only before Linux 2.6.0) or Waking (Linux 2.6.33 "
                             "to 3.13  only) ";
                break;
            case 'X':
                std::cout << "Dead (from Linux 2.6.0 onward) ";
                break;
            case 'x':
                std::cout << "Dead (Linux 2.6.33 to 3.13 only) ";
                break;
            case 'K':
                std::cout << "Wakekill (Linux 2.6.33 to 3.13 only) ";
                break;
            case 'P':
                std::cout << "Parked (Linux 3.9 to 3.13 only) ";
                break;
            default:
                std::cout << "Unknown - " << state << " ";
                break;
            }
        }
        std::cout << "\n";
    }
    return 0;
}

int main(int argc, char** argv)
{
    pid = getpid(); // use self pid as default value
    boost::program_options::variables_map vm;
    boost::program_options::options_description desc("Allowed options");
    add_options(desc);
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        std::cout << "Build with Compiler: " << BOOST_COMPILER << std::endl
                  << "Platform: " << BOOST_PLATFORM << std::endl
                  << "Library: " << BOOST_STDLIB << std::endl
                  << "Boost " << BOOST_LIB_VERSION << std::endl;
        return 0;
    }

    if (vm.count("man"))
    {
        std::cout << man_proc << std::endl;
        return 0;
    }

    std::vector<std::string> ignore_list{"fd", "net", "fdinfo", "ns", "attr"};
    std::stringstream strm;
    strm << "/proc/" << pid << "/task";

    using namespace boost::filesystem;
    auto prog_start_time = std::chrono::high_resolution_clock::now();
    try
    {
        recursive_directory_iterator dir(strm.str().c_str()), end;
        while (dir != end)
        {
            if (std::find(std::begin(ignore_list), std::end(ignore_list),
                          dir->path().filename()) != std::end(ignore_list))
            {
                dir.no_push();
            }

            if (dir->path().filename() == "stat")
                display_proc_stat(dir->path().string().c_str());

            if (dir->path().filename() == "stack" && vm.count("stack"))
            {
                std::ifstream f(dir->path().string().c_str());

                if (f.is_open())
                    std::cout << f.rdbuf() << std::endl;
                else
                    std::cout << "error open " << dir->path() << std::endl;
            }
            ++dir;
        }
    }
    catch (const std::exception& ex)
    {
        std::cout << "exception: " << ex.what() << std::endl;
        return 1;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout << argv[0] << " pid " << getpid() << " used "
              << std::chrono::duration_cast<std::chrono::milliseconds>(t1 -
                                                                       prog_start_time)
                     .count()
              << " milliseconds, sysconf (_SC_CLK_TCK) return " << sysconf(_SC_CLK_TCK)
              << std::endl;
    return 0;
}
