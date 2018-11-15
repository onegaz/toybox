#include <cstdio>
#include <vector>
#include <cinttypes>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <thread>
#include <memory>
#include <chrono>
#include <stdlib.h>
#include <algorithm>
#include <array>

const char* notes=R"(
make help
make user-ctor.s
g++ -std=c++11 -Wa,-adhln -g ../user-ctor.cpp > user-ctor.cpp.s

http://en.cppreference.com/w/cpp/language/move_assignment
user defined destructor prevents implicit move assignment

https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md
C.43: Ensure that a value type class has a default constructor
The default constructor is only auto-generated if there is no user-declared constructor

)";

class user_six
{
public:
    user_six(const char* msg)
    {
        std::snprintf(m_buf.data(), m_buf.size(), "%s ", msg);
        printf("%suser_six user declared ctor this=%p tid=%d\n", m_buf.data(), this,
               (int) syscall(SYS_gettid));
    }
    user_six()
    {
        printf("user_six ctor this=%p tid=%d\n", this, (int) syscall(SYS_gettid));
    }
    ~user_six()
    {
        printf("%suser_six dtor this=%p tid=%d\n", m_buf.data(), this,
               (int) syscall(SYS_gettid));
    }

    user_six(user_six&)
    {
        printf("user_six copy ctor this=%p tid=%d\n", this, (int)syscall(SYS_gettid));
    }
    user_six& operator=(const user_six&)
    {
        printf("user_six copy assignment operator= this=%p tid=%d\n", this, (int)syscall(SYS_gettid));
        return *this;
    }

    user_six(user_six&&)
    {
        printf("user_six move ctor this=%p\n", this);
    }
    user_six& operator=(user_six&&)
    {
        printf("user_six move assignment operator= this=%p tid=%d\n", this, (int)syscall(SYS_gettid));
        return *this;
    }
private:
	std::array<char, 128> m_buf={};
};

class user_ctor
{
public:
    user_ctor(int a, int b) : data(a + b)
    {
    	printf("user_ctor user-declared ctor this=%p tid=%d\n", this, (int)syscall(SYS_gettid));
    }
    user_ctor() =
        default; // C.43: Ensure that a value type class has a default constructor
    const char* to_string()
    {
        snprintf(m_buf, sizeof(m_buf), "user_ctor:%d this:%p", data, this);
        return m_buf;
    }
    int data = 3;
    char m_buf[128];
    user_six m_us;
};

class user_dtor
{
public:
	~user_dtor()
    {
    	printf("user_dtor user-declared dtor this=%p tid=%d\n", this, (int)syscall(SYS_gettid));
    }
    user_six m_us;
};

void test_default_ctor()
{
    printf("%s start\n", __func__);
    {
        std::vector<user_ctor> myvec(2);
        for (auto& data : myvec)
            printf("%s\n", data.to_string());
    }
    printf("%s end\n", __func__);
}

void test_move_ctor()
{
    printf("%s start\n", __func__);
    {
        user_ctor dd = user_ctor(4, 5);
        printf("%s static_cast<user_ctor&&>\n", __func__);
        user_ctor object = static_cast<user_ctor&&>(dd); // user_six move ctor this=0x7ffdba10daa4
        printf("std::move temp obj\n");
        user_ctor lval2 = std::move(user_ctor(4, 5)); // user_six move ctor this=0x7fff432397e4
    }
    {
    	printf("vector<user_six>\n");
        std::vector<user_six> vec;
        vec.push_back(user_six());
        printf("clear vec\n");
        vec.clear();
        printf("emplace_back vec\n");
        vec.emplace_back(user_six());
    }
    printf("%s end\n", __func__);
}

void test_move_assign()
{
    printf("%s start\n", __func__);
    {
    	printf("user_six\n");
    	user_six lval, obj;
    	lval = std::move(obj); // user_six move assignment operator= this=0x7ffd1372408e
    }
    {
    	printf("user_ctor\n");
    	user_ctor lval, obj(5, 5);
    	lval = std::move(obj); // user_six move assignment operator= this=0x7ffd13724114
    }
    {
    	printf("user_dtor prevents implicit move assignment\n");
    	user_dtor lval, obj;
    	lval = std::move(obj); // user_six copy assignment operator= this=0x7ffd1372408e
    }
    printf("%s end\n", __func__);
}

uint64_t get_stack_ptr(void)
{
    __asm__("mov %rsp, %rax");
}

// https://linux.die.net/man/3/end
extern void* _etext;
extern void* _edata;
extern void* _end;

void thread_proc()
{
	printf("tid %d stack pointer 0x%" PRIX64 ", "
			"sbrk(0) return %p\n", (int)syscall(SYS_gettid),
			get_stack_ptr(), sbrk(0));
    user_six stackobj("stack");
    std::unique_ptr<user_six> heapobj(new user_six("heap"));
    srand (time(nullptr));
    std::this_thread::sleep_for(std::chrono::milliseconds(900+rand() % 100));
}

int main()
{
	printf("tid %d stack pointer 0x%" PRIX64 ", "
			"sbrk(0) return %p\n", (int)syscall(SYS_gettid),
			get_stack_ptr(), sbrk(0));
	printf("the first address past the end of the text segment (the program code):%p\n", &_etext);
	printf("the first address past the end of the initialized data segment:%p\n", &_edata);
	printf("the first address past the end of the uninitialized data segment (also known as the BSS segment):%p\n", &_end);

	std::vector<std::thread> v;
	for(int i=0; i<4; ++i)
		v.emplace_back(std::thread(thread_proc));
	std::for_each(v.begin(), v.end(), [](std::thread &thrd){ thrd.join(); });
	printf("after multiple threads stopped\n\n");

    test_move_ctor();
    test_move_assign();
    test_default_ctor();
    printf("stack pointer 0x%" PRIX64 ", heap is around %p sbrk(0) return %p\n",
    			get_stack_ptr(), &_end, sbrk(0));
    return 0;
}
