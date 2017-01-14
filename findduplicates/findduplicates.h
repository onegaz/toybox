#include <array>
#include <string>
#include <openssl/sha.h>
#include <unordered_set>
#include <thread>
typedef std::array<unsigned char, SHA_DIGEST_LENGTH> sha_digest_t;

int sha1checksum(const std::string& filename, sha_digest_t& digest);
void find_duplicates_by_digest(std::unordered_set<std::string>& inputpaths);
std::ostream& getostream();

class MyApp: public wxApp
{
public:
	class wxFrame* m_frame;
    bool m_cancel_operation = false;
    bool OnInit();
    void Cancel();
    std::thread m_workthread;
    std::unordered_set<std::string> inputpaths;
    std::string duplicatedfiles;
    size_t total_file_cnt = 0;
};
DECLARE_APP(MyApp)
