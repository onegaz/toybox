#include <algorithm>
#include <chrono>
#include <iterator>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <unordered_set>
#include <boost/tokenizer.hpp>
#include <boost/program_options.hpp>

// return "" if PID is not found in specified pid_column
std::string extract_pid_from_logcat(const std::string &line, int pid_column)
{
    // 09-19 21:37:45.500 11406 12089 I TagName: #onEndOfData
	// pid_column is 3 for above example
    boost::char_separator<char> sep(" \t");
    boost::tokenizer<boost::char_separator<char>> tokens(line, sep);
    boost::tokenizer<boost::char_separator<char>>::iterator itEnd = tokens.end();
    int i                                                         = 0;
    for (boost::tokenizer<boost::char_separator<char>>::iterator it = tokens.begin(); it != itEnd;
         ++it)
    {
        i++;
        if (i == pid_column)
        {
            std::string pid_candidate = *it;
            if (pid_candidate.find_first_not_of("0123456789") == std::string::npos)
                return pid_candidate;
            return "";
        }
    }
    return "";
}

struct FilterLogcatApp {
	std::string m_logcat_file_path;
	std::vector<std::string> m_plain_patterns;
	std::vector<std::string> m_exclude_patterns;
	boost::program_options::variables_map vm;
	boost::program_options::options_description desc;
	std::chrono::high_resolution_clock::time_point process_start_time = std::chrono::high_resolution_clock::now();
	int m_pid_max_pos = 24;
	int m_pid_min_pos = 18;
	int m_pid_column = 3;

	FilterLogcatApp():desc("Allowed options")
	{

	}

	bool ParseArgs(int argc, char* argv[])
	{
		// clang-format off
		desc.add_options()("help,h", "produce help message\n"
				"Find pid associated with specified keywords and extract all messages from those pids, but ingore lines containing keywords to exclude")
		("file,f", boost::program_options::value<decltype(m_logcat_file_path)>(&m_logcat_file_path), "logcat file path")
		("patterns,p", boost::program_options::value<decltype(m_plain_patterns)>(&m_plain_patterns)->multitoken(), "find PID containing plain text patterns")
		("exclude,x", boost::program_options::value<decltype(m_exclude_patterns)>(&m_exclude_patterns)->multitoken(), "exclude lines containing text patterns")
		("pid_max_pos", boost::program_options::value<decltype(m_pid_max_pos)>(&m_pid_max_pos)->default_value(m_pid_max_pos), "PID within first N characters")
		("pid_min_pos", boost::program_options::value<decltype(m_pid_min_pos)>(&m_pid_min_pos)->default_value(m_pid_min_pos), "PID after first N characters")
		("pid_column", boost::program_options::value<decltype(m_pid_column)>(&m_pid_column)->default_value(m_pid_column), "PID column number in logcat message")
		  ;
		// clang-format on

		try
		{
			boost::program_options::store(
				boost::program_options::parse_command_line(argc, argv, desc), vm);
			boost::program_options::notify(vm);
		}
		catch (const std::exception &ex)
		{
			std::cerr << ex.what() << std::endl;
		}
		return !m_logcat_file_path.empty() && !m_plain_patterns.empty();
	}

	int GetDurationMilliseconds()
	{
		auto process_end_time = std::chrono::high_resolution_clock::now();
		auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(process_end_time -
																			process_start_time);
		std::chrono::duration<double, std::milli> fp_ms = process_end_time - process_start_time;
		return fp_ms.count();
	}

	bool CheckPidPosition(int pid_pos)
	{
		return pid_pos != std::string::npos && pid_pos >= m_pid_min_pos &&
			   pid_pos <= m_pid_max_pos;
	}
};

int main(int argc, char *argv[])
{
    FilterLogcatApp myApp;

    if (!myApp.ParseArgs(argc, argv))
    {
        std::cerr << myApp.desc << std::endl;
        return myApp.vm.count("help") ? 0 : 1;
    }

    std::vector<std::string> lines;
    std::unordered_set<std::string> pids;

    try
    {
		std::ifstream logcatfile(myApp.m_logcat_file_path.c_str());
		for (std::string line; getline(logcatfile, line);)
		{
			lines.push_back(line);
		}
		logcatfile.close();
    } catch(const std::exception &ex)
    {
    	std::cerr << argv[0] << " failed to read file " << myApp.m_logcat_file_path << "\n";
    	std::cerr << ex.what() << std::endl;
    	return 1;
    }

    // find PIDs associated with specified keywords
    for (auto line : lines)
    {
        if (std::any_of(
                myApp.m_plain_patterns.begin(), myApp.m_plain_patterns.end(),
                [&line](std::string keywd) { return line.find(keywd) != std::string::npos; }))
        {
            auto pid_candidate = extract_pid_from_logcat(line, myApp.m_pid_column);
            if (pid_candidate.length() > 0)
                pids.insert(" " + pid_candidate + " ");
        }
    }

    // find lines associated with interested PIDs
    int line_count     = 0;
    int excluded_count = 0;
    for (auto line : lines)
    {
        if (std::any_of(pids.begin(), pids.end(), [&line, &myApp](std::string pid) {
                return myApp.CheckPidPosition(line.find(pid));
            }))
        {
            if (std::any_of(myApp.m_exclude_patterns.begin(), myApp.m_exclude_patterns.end(),
                            [line](std::string exclude_pattern) {
                                return line.find(exclude_pattern) != std::string::npos;
                            }))
            {
                ++excluded_count;
                continue;
            }

            line_count++;
            std::cout << line << std::endl;
        }
    }

    // statistics of the result
    std::cout << "Excluded " << excluded_count << " lines containing keywords to exclude, ";
    std::cout << "found " << line_count << " lines out of " << lines.size() << " from "
              << myApp.m_logcat_file_path << std::endl;
    std::cout << "Process took " << myApp.GetDurationMilliseconds() << " milliseconds\n";
    if (!pids.empty())
    {
        std::cout << "Found pids containing specified keywords:";
        std::copy(pids.begin(), pids.end(), std::ostream_iterator<std::string>(std::cout, ""));
        std::cout << std::endl;
    }
    return 0;
}
