
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <wx/frame.h>
#include <wx/app.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/clipbrd.h>
#include <wx/msgdlg.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/null.hpp>
#include <vector>
#include <unordered_map>
#include "findduplicates.h"
#include "mainwnd.h"

int sha1checksum(const std::string& filename, sha_digest_t& digest) {
	memset(digest.data(), 0, digest.size());
	std::array< char, 1024*128> buf;
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    std::ifstream ifs (filename.c_str(),std::ios_base::binary);
    if (!ifs.good()) {
    	std::cout << __FUNCTION__ << " errno=" << errno << filename << std::endl;
    	return errno;
    }
    while(!ifs.eof()) {
    	size_t extracted = ifs.read(buf.data(), buf.size()).gcount();
    	if(extracted>0) {
    		SHA1_Update(&ctx, buf.data(), extracted);
    	}
    }
    SHA1_Final(digest.data(), &ctx);
    return 0;
}

bool string_to_digest(const std::string& input,
		sha_digest_t& output) {
	memset(output.data(), 0, output.size());
    static const char* const lut = "0123456789abcdef"; // "0123456789ABCDEF";
    size_t len = input.length();
    if (len!=output.size()*2) {
    	std::cout << __FUNCTION__ << " unexpected digest string length " << len << std::endl;
    	return false;
    }
    if (len & 1) throw std::invalid_argument("odd length");

    for (size_t i = 0; i < len; i += 2)
    {
        char a = input[i];
        const char* p = std::lower_bound(lut, lut + 16, a);
        if (*p != a) throw std::invalid_argument("not a hex digit");

        char b = input[i + 1];
        const char* q = std::lower_bound(lut, lut + 16, b);
        if (*q != b) throw std::invalid_argument("not a hex digit");
        output[i/2] = (((p - lut) << 4) | (q - lut));
    }
    return true;
}

boost::iostreams::stream< boost::iostreams::null_sink > nullOstream( ( boost::iostreams::null_sink() ) );
int verbose=0;

std::ostream& getostream() {
	if(verbose)
		return std::cout;
	return nullOstream; // another not so good option: std::cout.setstate(std::ios_base::badbit);
}

bool MyApp::OnInit()
{
	m_frame = new MyFrame();
	m_frame->Show();
	return true;
}

IMPLEMENT_APP(MyApp)

void MyApp::Cancel() {
	m_cancel_operation = true; //
	if(m_workthread.joinable())
		m_workthread.join();
	m_cancel_operation = false;
}
