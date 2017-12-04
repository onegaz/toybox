#include <vector>
#include <type_traits>

template <typename T>
struct imgvec
{
    unsigned int n = 0;
    T* store = nullptr;
    void reserve(uint32_t sz)
    {
        if (sz == n)
            return;
        store = new T[sz];
        n = sz;
    }
    T get(uint32_t k) const
    {
        return store[k];
    }
    void set(T* src, uint32_t cnt)
    {
        reserve(cnt);
        std::memcpy(store, src, cnt * sizeof(T));
    }
    unsigned int size() const
    {
        return n;
    }

    double cosine_similarity(const imgvec<T>& rhs) const
    {
        double dot = 0.0;

        for (unsigned int i = 0u; i < size(); ++i)
        {
            dot += store[i] * rhs.store[i];
        }
        return dot / (length() * rhs.length());
    }

    double length() const
    {
        double denom_a = 0.0;
        for (unsigned int i = 0u; i < size(); ++i)
        {
            denom_a += store[i] * store[i];
        }
        return sqrt(denom_a);
    }

    double similarity(const imgvec<T>& rhs) const
    {
        double sim = cosine_similarity(rhs);
        double slen = length();
        double rlen = rhs.length();
        sim *= 4 * slen * rlen / ((slen + rlen) * (slen + rlen));
        return sim;
    }

    double euclidean_distance(const imgvec<T>& rhs) const
    {
        double sim = 0;
        for (size_t i = 0; i < size(); ++i)
        {
            auto comp_diff = store[i] - rhs.store[i];
            sim += comp_diff*comp_diff;
        }

        return sqrt(sim);
    }

    imgvec(char* src, uint32_t len)
    {
        reserve(len);
        memcpy(store, src + sizeof(n), len * sizeof(T));
    }
};


template <typename T>
int read_sift_vec_file(const std::string& filename, std::vector<imgvec<T>>& imgs)
{
    std::ifstream srcfile(filename.c_str(), std::ios::binary);
    // http://corpus-texmex.irisa.fr/ivecs_read.m
    uint32_t d = 0;
    srcfile.read(reinterpret_cast<char*>(&d), sizeof(d));
    uint32_t vecsizeof = 1 * 4 + d * sizeof(T);
    srcfile.seekg(0, std::ios_base::end);
    auto size = srcfile.tellg();
    srcfile.seekg(0, std::ios_base::beg);
    size_t numvec = size / vecsizeof;
    std::array<char, 1024*256> buf; // Assume one vector can fit into this buffer
    if (vecsizeof>buf.size())
    {
    	std::snprintf(buf.data(), buf.size(),
    			"Vector size %d > buffer size %zu, please increase buf size if vector size is valid",
				vecsizeof, buf.size());
    	throw std::out_of_range(buf.data());
    }

    for (size_t i = 0; i < numvec; ++i)
    {
        srcfile.read(buf.data(), vecsizeof);
        imgs.emplace_back(buf.data(), d);
    }
    return imgs.size();
}

template <typename T>
void print_some(const std::vector<imgvec<T>>& imgs, size_t rows, size_t cols, const std::vector<size_t>& row_idx)
{
	auto print_one_vec=[=](const imgvec<T>& row)
		{
    	std::cout << row.size() << "\t";
    	for (size_t j=0; j<cols; ++j)
    		std::cout << row.get(j) << "\t";
    	std::cout << std::endl;
		};

	if (row_idx.size())
	{
		for (auto i:row_idx)
		{
			print_one_vec(imgs[i]);
		}
	}
	else
    for (size_t i=0; i<std::min(rows, imgs.size()); ++i)
    {
    	print_one_vec(imgs[i]);
    }
    std::cout << "There are " << imgs.size() << " records\n";
}
