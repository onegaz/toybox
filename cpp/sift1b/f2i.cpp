#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <array>
#include <cstring>
#include <cstdint>
#include <math.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <boost/config.hpp>
#include <boost/program_options.hpp>
#include "imgvec.hpp"

void add_options(boost::program_options::options_description& desc)
{
    desc.add_options()
    	("help,h", "Produce help message")
    	("file,f", boost::program_options::value<std::string>(),"specify file name")
		("corpus", boost::program_options::value<std::string>(),"specify corpus file name")
		("query_file", boost::program_options::value<std::string>(),"specify query file name")
		("queryidx", boost::program_options::value<std::vector<size_t>>(),"specify row number to query")
		("rows,r", boost::program_options::value<size_t>(),"specify number of rows")
		("cols,c", boost::program_options::value<size_t>(),"specify number of columns to print")
		("row", boost::program_options::value<std::vector<size_t>>(),"specify row number to print")
		("type,t", boost::program_options::value<std::string>(),"specify data type of components: int, float, byte")
		;
}

void query(const std::string& corpus_file, const std::string& queryfile, size_t queryidx)
{
    std::vector<imgvec<float>> corpus_vec, qry_vec;
    read_sift_vec_file<float>(corpus_file, corpus_vec);
    read_sift_vec_file<float>(queryfile, qry_vec);
    imgvec<float> qry = qry_vec[queryidx];
    std::vector<std::pair<double, size_t>> euclidean_dist;
    for (size_t i=0; i<corpus_vec.size(); ++i)
    {
    	double dist = 0;
    	dist = qry.euclidean_distance(corpus_vec[i]);
    	dist = qry.cosine_similarity(corpus_vec[i]);
//    	dist = qry.similarity(corpus_vec[i]);
    	euclidean_dist.emplace_back(dist, i);
    }
    std::sort(euclidean_dist.begin(), euclidean_dist.end(), [](const std::pair<double, size_t>& lhs,
    		const std::pair<double, size_t>& rhs){
    	return lhs.first < rhs.first;
    });
    std::cout << "Query result: ";
    std::vector<size_t> sorted_idx;
    for (size_t i=0; i<32; ++i)
    {
    	std::cout << euclidean_dist[i].second
//		<< "," <<euclidean_dist[i].first
		<< "\t";
    }
    std::cout << std::endl;

    for (size_t i=0; i<100; ++i)
    	sorted_idx.push_back(euclidean_dist[i].second);

    std::cout << "Sorted result: ";
    std::sort(sorted_idx.begin(), sorted_idx.end());
    for (size_t i=0; i<32; ++i)
    {
    	std::cout << sorted_idx[i]		<< "\t";
    }
    std::cout << std::endl;
}

int main(int argc, char** argv)
{
	std::string corpus_file = "siftsmall_base.fvecs";
	std::string query_file = "siftsmall_query.fvecs";
	size_t queryidx = 0;
	query(corpus_file, query_file, queryidx);
	return 0;
    boost::program_options::variables_map vm;
    boost::program_options::options_description desc("Allowed options");
    add_options(desc);
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help") || vm.count("file")==0 || vm.count("type")==0)
    {
        std::cout << desc << std::endl;
        std::cout << "Build with Compiler: " << BOOST_COMPILER << std::endl
                  << "Platform: " << BOOST_PLATFORM << std::endl
                  << "Library: " << BOOST_STDLIB << std::endl
                  << "Boost " << BOOST_LIB_VERSION << std::endl;
        return 0;
    }

    if (vm.count("corpus") && vm.count("query_file"))
    {
    	std::string corpus_file = vm["corpus"].as<std::string>();
    	std::string query_file = vm["query_file"].as<std::string>();
    	query(corpus_file, query_file, queryidx);
    	return 0;
    }

    std::string ivecs_filename = vm["file"].as<std::string>();
    size_t rows = 4;
    size_t cols = 16;
    std::vector<size_t> row_idx;
    if (vm.count("rows"))
    	rows = vm["rows"].as<size_t>();
    if (vm.count("row"))
    	row_idx = vm["row"].as<std::vector<size_t>>();
    if (vm.count("cols"))
    	cols = vm["cols"].as<size_t>();
    std::string component_type = "float";

    if (vm.count("type"))
    {
    	if (vm["type"].as<std::string>()=="int")
    	{
    	    std::vector<imgvec<int>> imgs;
    	    read_sift_vec_file<int>(ivecs_filename, imgs);
    	    print_some<int>(imgs, rows, cols, row_idx);
    	}
    	if (vm["type"].as<std::string>()=="float")
    	{
    	    std::vector<imgvec<float>> imgs;
    	    read_sift_vec_file<float>(ivecs_filename, imgs);
    	    print_some<float>(imgs, rows, cols, row_idx);
    	}
    	if (vm["type"].as<std::string>()=="byte")
    	{
    	    std::vector<imgvec<uint8_t>> imgs;
    	    read_sift_vec_file<uint8_t>(ivecs_filename, imgs);
    	    print_some<uint8_t>(imgs, rows, cols, row_idx);
    	}
    }

    return 0;
}

const char* mynotes=R"mynotes(
download dataset from http://corpus-texmex.irisa.fr/
wget ftp://ftp.irisa.fr/local/texmex/corpus/siftsmall.tar.gz

			format of siftsmall_groundtruth.ivecs
vector[0]	dimension	components[0] ... components[dimension-1]
vector[1]	dimension	components[0] ... components[dimension-1]
...
vector[99]	dimension	components[0] ... components[dimension-1]

)mynotes";
