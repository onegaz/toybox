#include <mutex>
#include <thread>
#include <iostream>
#include <vector>
#include <functional>
#include <chrono>
#include <string>
#include <sstream>
#include <link.h>
#include <type_traits>
#include <boost/type_index.hpp>
#include <boost/config.hpp>
#include <boost/version.hpp>

template<typename T>
struct showme{
    std::string func_;
    T* obj_;
    showme(const std::string& func, T* obj):func_(func), obj_(obj){}
    ~showme() {            
        std::cout << func_ << " " << obj_->to_string() << std::endl;
    }
};

struct mylocktype { // This class is used check how std::lock works
    int try_count = 0;
    bool locked=false;
    std::string name_;
    mylocktype(const std::string& name): name_(name){}
    bool try_lock() {
        showme<mylocktype> show_before_exit(__func__, this);
        try_count++;
        if(try_count<4)
            return false;
        locked = true;
        return true;
    }
    void lock() {
        locked = true;
        try_count = 0;
        showme<mylocktype> show_before_exit(__func__, this);
    }
    void unlock() {
        locked = false;
        try_count = 0;
        showme<mylocktype> show_before_exit(__func__, this);
    }
    std::string to_string() const {
        std::stringstream ss;
        ss << "mylocktype "<< name_ <<" try_count="<<try_count<< " locked=" << std::boolalpha << locked << " this=" << this;
        return ss.str();
    }
};

template<typename Lockable , typename... Rest> 
struct lock_guard_multi_lockableptr {
    std::vector<Lockable> group_;
    // void operator()(Lockable lk, Rest... rest) {
    //     static_assert(std::is_pointer<Lockable>::value, "type must be Lockable pointer");
    //     group_={rest...};
    //     group_.push_back(lk);
    //     std::lock(*lk, *rest...);
    // }
    lock_guard_multi_lockableptr(Lockable lk, Rest... rest) {
        static_assert(std::is_pointer<Lockable>::value, "type must be Lockable pointer");
        group_={lk, rest...};
        std::lock(*lk, *rest...);        
    }
    ~lock_guard_multi_lockableptr() {
        for(auto& lk:group_)
            lk->unlock();
    }
};

template<typename Lockable , typename... Rest> 
struct lock_guard_multi_lockableref {
    std::vector<Lockable*> group_;
    lock_guard_multi_lockableref(Lockable& lk, Rest&... rest) {
        static_assert(std::is_reference<decltype(lk)>::value, "type must be Lockable reference");
        group_={&lk, &rest...};
        std::lock(lk, rest...);        
    }
    ~lock_guard_multi_lockableref() {
        for(auto& lk:group_)
            lk->unlock();
    }
};

static int dl_iterate_phdr_callback(struct dl_phdr_info *info, size_t size, void *data)
{
    std::cout << info->dlpi_name << "\t(" << info->dlpi_phnum << " segments)" << std::endl;
    return 0;
}
int main() {
    mylocktype lk1{"lk1"}, lk2{"lk2"}, lk3{"lk3"}, lk4{"lk4"};
    {
    std::lock(lk1, lk2);
    std::cout << "after lock lk1 and lk2" << std::endl;
    std::lock_guard<mylocktype> lock_a(lk2, std::adopt_lock);
    std::lock_guard<mylocktype> lock_b(lk1, std::adopt_lock);
    }
    std::cout << "try lock in different order\n\n";
    {
    lock_guard_multi_lockableptr<mylocktype*, mylocktype*, mylocktype*> holder{&lk3, &lk4, &lk2};
    std::cout << "after " << boost::typeindex::type_id_with_cvr<decltype(holder)>().pretty_name() << std::endl;
    }
    {
    lock_guard_multi_lockableref<mylocktype, mylocktype, mylocktype> holder{lk3, lk4, lk1};
    std::cout << "after " << boost::typeindex::type_id_with_cvr<decltype(holder)>().pretty_name() << std::endl;
    }
    dl_iterate_phdr(dl_iterate_phdr_callback, NULL);
    std::cout << "Built with "<< BOOST_COMPILER <<" STDLIB " << BOOST_STDLIB << " on platform " << BOOST_PLATFORM 
        <<" with boost " << BOOST_VERSION << std::endl;
    return 0;
}

/*

rm CMakeCache.txt && cmake . && make

*/