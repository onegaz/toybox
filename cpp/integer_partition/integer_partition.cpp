#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>
#include <boost/program_options.hpp>

bool verbose_mode=true;

void partition_integer(int num, int pos, std::vector<int>& result, int& cnt)
{
    if (pos < 1)
        return;
    int groups = result.size();
    int min_val = result[pos - 1];
    int sum = std::accumulate(result.begin(), result.begin() + pos, 0);
    if (sum + min_val * (groups - pos) > num)
        return;

    if (pos == result.size() - 1)
    {
        int value = num - sum;
        if (value >= min_val)
        {
            ++cnt;
            result[pos] = value;
            if (verbose_mode)
            {
                std::copy(result.begin(), result.end(),
                          std::ostream_iterator<int>(std::cout, " "));
                std::cout << "\n";
            }
            return;
        }
        return;
    }

    for (int i = min_val; i <= (num-sum)/(groups - pos); i++)
    {
        result[pos] = i;
        partition_integer(num, pos + 1, result, cnt);
    }

    return;
}

int main(int argc, char* argv[])
{
    int num = 10;
    int groups = 3;
    int count = 0;
    boost::program_options::variables_map vm;
    boost::program_options::options_description desc;
    namespace po = boost::program_options;
    desc.add_options()
      ("help,h", "Print help messages")
	  ("verbose", "Enable verbose mode: print partitions")
      ("num", po::value<int>(&num),  "integer to partition")
	  ("groups", po::value<int>(&groups),  "groups")
	  ;

    try
    {
        po::store(
            po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(),
            vm);
        po::notify(vm);

        verbose_mode = vm.count("verbose");

        if (vm.count("help"))
        {
            std::cout << argv[0] << " usage:" << std::endl << desc << std::endl;
            return 0;
        }
    }
    catch (po::error& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;
        return __LINE__;
    }

    std::vector<int> result(groups);
    std::fill(result.begin(), result.end(), 0);
    for (int i = 1; i <= num/groups; i++)
    {
        result[0] = i;
        partition_integer(num, 1, result, count);
    }

    std::cout << "There are " << count << " ways to partition " << num << " into "
              << groups << " groups\n";
    return 0;
}
