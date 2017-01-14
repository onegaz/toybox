#include <iostream>
#include <utility>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/range/iterator_range.hpp>
#include <wx/frame.h>
#include <wx/app.h>
#include "findduplicates.h"

std::string digest_to_string(const sha_digest_t& digest) {
    std::string sha1str;
    std::stringstream ss;
    for(auto data: digest)
   	 ss << std::hex <<std::setw(2)<<std::setfill('0') << (int)data;
    sha1str = ss.str();
    return sha1str;
}

void find_duplicates_by_digest(std::unordered_set<std::string>& inputpaths) {
	std::unordered_set<std::string> visited;
	std::unordered_multimap<std::string, std::string> filedigestdb; // key: digest, value: path
	wxGetApp().total_file_cnt = 0;
	// send to display when found
	auto process_file = [&](const std::string& onepath) {
		if(visited.count(onepath)==0) {
			wxGetApp().total_file_cnt++;
			visited.insert(onepath);
			sha_digest_t digest;
			sha1checksum(onepath, digest);
			std::string sdigest = digest_to_string(digest);
			filedigestdb.insert(std::pair<std::string, std::string>{sdigest, onepath});
		}
	};

	std::function<void (const std::string& onepath)> process_dir;
	process_dir = [&](const std::string& onepath) {
		if(wxGetApp().m_cancel_operation) {
			std::cout << "Operation is cancelled" << std::endl;
			return;
		}

		if(visited.count(onepath)==0) {
			visited.insert(onepath);
		} else
			return;
		boost::filesystem::path fspath(onepath);
		for(auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(fspath), {})) {
//			std::cout << entry << "\n";
			if (boost::filesystem::is_regular_file(entry))  {
						process_file(entry.path().string());
					}
			else {
				// process_dir(entry.path().string());
				inputpaths.insert(entry.path().string());
			}
		}
	};
	auto t1 = std::chrono::high_resolution_clock::now();
	while(inputpaths.size()) {
		if(wxGetApp().m_cancel_operation) {
			std::cout << "Operation is cancelled" << std::endl;
			break;
		}
		try {	// avoid setup exception handler many times.
			while(inputpaths.size()) {
				if(wxGetApp().m_cancel_operation) {
					std::cout << "Operation is cancelled" << std::endl;
					break;
				}
				std::string onepath = *inputpaths.begin();
				inputpaths.erase(onepath);
				boost::filesystem::path fspath(onepath);
				if (boost::filesystem::is_directory(fspath))  {
					process_dir(onepath);
				}

				if (boost::filesystem::is_regular_file(fspath))  {
					process_file(onepath);
				}
			}

		} catch (std::exception& ex) {
			std::cout << __FUNCTION__ << " " << ex.what() << std::endl;
		}
	}
	visited.clear();
	std::stringstream ss;
	std::string lastdigest;
	size_t duplicatedcnt = 0;
	for(auto fd: filedigestdb) {
		if(filedigestdb.count(fd.first)>1) {
			if(lastdigest != fd.first ) {
				ss << "Duplicated file"<< std::endl;
				lastdigest = fd.first;
			}
			ss << fd.second << std::endl;
			duplicatedcnt++;
		}
	}
    auto t2 = std::chrono::high_resolution_clock::now();
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    std::chrono::duration<double, std::milli> fp_ms = t2 - t1;

	ss << "Found " << duplicatedcnt << " duplicated files from "<< wxGetApp().total_file_cnt << " files in " << int_ms.count() << " milliseconds" << std::endl;
	wxGetApp().duplicatedfiles = ss.str();
	wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, 0);
    event.SetEventObject( wxGetApp().m_frame );
    event.SetString( ss.str() );
    wxPostEvent(wxGetApp().m_frame , event);
}
