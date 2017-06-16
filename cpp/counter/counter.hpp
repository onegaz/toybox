#ifndef __counter_hpp__
#define __counter_hpp__
#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <boost/current_function.hpp>

template<int N>
struct crtp_counter_store
{
	size_t store;
	crtp_counter_store(const char* FN)
	{
		store = 0;
		reg(FN, &store);
	}

	static void reg(const char* FN, size_t* cnt)
	{
		int pos = next();
		get_counter_pointers()[pos] = cnt;
		sprintf(get_counter_name(pos), "%s", FN);
	}

	static size_t** get_counter_pointers()
	{
		static size_t* ptrs[N];
		return ptrs;
	}

	static char* get_counter_name(int n)
	{
		static char names[N][256];
		return names[n];
	}

	static int next()
	{
		static int next_pos=0;
		return __sync_fetch_and_add(&next_pos, 1);
	}

	static void dump()
	{
		int i = 0;
		size_t** cnts = get_counter_pointers();
		int name_col_width = 8*2;
		write(STDOUT_FILENO, "Type", strlen("Type"));
		int tabs = name_col_width/8 - strlen(get_counter_name(i))/8;
		for(int i=0; i<tabs; i++)
			write(STDOUT_FILENO, "\t", 1);
		printf("Counter\n");
		for (int i=0; i<N; i++)
		{
			if (cnts[i])
			{
				write(STDOUT_FILENO, get_counter_name(i), strlen(get_counter_name(i)) );
				int tabs = name_col_width/8 - strlen(get_counter_name(i))/8;
				for(int i=0; i<tabs; i++)
					write(STDOUT_FILENO, "\t", 1);
				printf("%zu\n", *cnts[i]);
			}
		}
	}
};

template <typename T>
struct crtp_counter
{
	crtp_counter()
    {
    	__sync_fetch_and_add( &get_counter(), 1 );
    }

    static size_t& get_counter()
    {
    	static crtp_counter_store<2048> ccs(get_type_name());
    	return ccs.store;
    }

    ~crtp_counter()
    {
    	__sync_fetch_and_sub( &get_counter(), 1 );
    }

    static const char* get_type_name(void)
    {
    	static const char gcc_prefix[] = "static const char* crtp_counter<T>::get_type_name() [with T = ";
    	static const char gcc_postfix[] = "]";
    	static const char clang_prefix[] = "static const char *crtp_counter<";
    	static const char clang_postfix[] = ">::get_type_name()";
    	static char name[sizeof(BOOST_CURRENT_FUNCTION)-sizeof(clang_prefix)];
    	if (!name[0])
    	{
    		// printf("%s\n", BOOST_CURRENT_FUNCTION);
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
    	}
    	return name;
    }
};

#endif
