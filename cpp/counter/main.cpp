#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <boost/version.hpp>
#include <boost/config.hpp>
#include "counter.hpp"

class Base : public crtp_counter<Base>
{
public:
	Base(){};

};

class Derived : public crtp_counter<Derived>, public Base
{
public:
	Derived(){}

};

class Derived2 : public crtp_counter<Derived2>, public Derived
{
public:
	Derived2(){}
};

using namespace std;

int main()
{
	vector<Base*> base_objs;
	vector<Derived*> derived_objs;
	vector<Derived2*> derived2_objs;
	for(int i=0; i<4; i++)
		base_objs.push_back(new Base);
	for(int i=0; i<6; i++)
		derived_objs.push_back(new Derived);
	for(int i=0; i<8; i++)
		derived2_objs.push_back(new Derived2);

	cout << "Object counters after ctor\n";
	crtp_counter_store_type::output(std::cout);
	std::for_each(base_objs.begin(), base_objs.end(), std::default_delete<Base>());
	std::for_each(derived_objs.begin(), derived_objs.end(), std::default_delete<Derived>());
	std::for_each(derived2_objs.begin(), derived2_objs.end(), std::default_delete<Derived2>());
	cout << "Object counters after dtor\n";
	crtp_counter_store_type::output(std::cout);

	cout  << "Use Curiously Recurring Template Pattern (CRTP) to count live objects\n"
		  << "Build with Compiler: " << BOOST_COMPILER << std::endl
		  << "Platform: " << BOOST_PLATFORM << std::endl
		  << "Library: " << BOOST_STDLIB << std::endl
		  << "Boost " << BOOST_LIB_VERSION << std::endl
		  ;
	return 0;
}
