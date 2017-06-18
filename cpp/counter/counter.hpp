#ifndef __counter_hpp__
#define __counter_hpp__
#include <string.h>
#include <boost/current_function.hpp>
#include <iostream>

struct object_counter
{
	char name[120];
	size_t count;
	bool empty() const
	{
		return name[0]==0;
	}
	void inc()
	{
		__sync_fetch_and_add(&count, 1);
	}
	void dec ()
	{
		__sync_fetch_and_sub(&count, 1);
	}
};

template<int N>
struct crtp_counter_store
{
	int index;
	crtp_counter_store(const char* FN)
	{
		static int next_pos=0;
		index = __sync_fetch_and_add(&next_pos, 1);
		if (index >= N)
			index = N - 1;
		strncpy(get_counter().name, FN, sizeof(get_counter().name)-1);
	}

	static object_counter& get_counter(int n)
	{
		static object_counter store[N];
		return store[n];
	}

	object_counter& get_counter()
	{
		return get_counter(index);
	}

    static void output(std::ostream& os)
    {
		int i = 0;
		int name_col_width = 8*2;
		os << "Type";
		int tabs = name_col_width/8 - strlen("Type")/8;
		for(int i=0; i<tabs; i++)
			os << "\t";
		os << "Counter\n";
    	for (int i=0; i<N; i++)
    	{
			if (get_counter(i).empty())
				break;
			os << get_counter(i).name;
			tabs = name_col_width/8 - strlen(get_counter(i).name)/8;
			for(int j=0; j<tabs; j++)
				os << "\t";
			os << get_counter(i).count	<< "\n";
    	}
		if ( i >= N-1 )
			os <<"Some counters may be missing, please increase value of N!\n";
    }
};

typedef crtp_counter_store<2048> crtp_counter_store_type;

template <typename T>
struct crtp_counter
{
	crtp_counter()
    {
		get_store().get_counter().inc();
    }

    static crtp_counter_store_type& get_store()
    {
    	static crtp_counter_store_type ccs(get_type_name());
    	return ccs;
    }

    ~crtp_counter()
    {
    	get_store().get_counter().dec();
    }

    static const char* get_type_name(void)
    {
    	static const char gcc_prefix[] = "static const char* crtp_counter<T>::get_type_name() [with T = ";
    	static const char gcc_postfix[] = "]";
    	static const char clang_prefix[] = "static const char *crtp_counter<";
    	static const char clang_postfix[] = ">::get_type_name";
    	static char name[sizeof(BOOST_CURRENT_FUNCTION)-sizeof(clang_prefix)];
    	if (!name[0])
    	{
#ifdef _MSC_VER
        	static const char msvc_prefix[] = "const char *__cdecl crtp_counter<";
        	static const char msvc_postfix[] = ">::get_type_name(void)";
//    		_MSC_VER 1900	http://rextester.com/l/cpp_online_compiler_visual
//    		BOOST_CURRENT_FUNCTION const char *__cdecl crtp_counter<int>::get_type_name(void)
//    		__FUNCSIG__ const char *__cdecl crtp_counter<int>::get_type_name(void)
//    		const char *__cdecl crtp_counter<int>::get_type_name(void)
    		const char* start = strstr(__FUNCSIG__, msvc_prefix) + strlen(msvc_prefix);
    		const char* endpos = strstr(__FUNCSIG__, msvc_postfix);
    		int len = endpos - start;
    		memcpy(name, start, len);
    		name[len] = 0;
#else
    		if (strstr(BOOST_CURRENT_FUNCTION, gcc_prefix)==nullptr)
    		{
        		strcpy(name, BOOST_CURRENT_FUNCTION + sizeof(clang_prefix)-1);
        		char* endpos = strstr (name, clang_postfix);
        		endpos[0] = 0;
    		}
    		else
    		{
        		strcpy(name, BOOST_CURRENT_FUNCTION + sizeof(gcc_prefix)-1);
        		name[strlen(name)-strlen(gcc_postfix)] = 0;
    		}
#endif
    	}
    	return name;
    }
};

#endif
