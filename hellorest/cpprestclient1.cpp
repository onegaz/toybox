#include <cpprest/http_client.h>
#include <cpprest/json.h>           
#include <cpprest/uri.h>   
#include <iostream>
#include <link.h>
#include <boost/program_options.hpp>
static int dl_iterate_phdr_callback(struct dl_phdr_info *info, size_t size, void *data)
{
    std::cout << info->dlpi_name << "\t(" << info->dlpi_phnum << " segments)" << std::endl;
    return 0;
}

void test(const std::string& uristr) {
	web::http::client::http_client client(utility::conversions::to_string_t(uristr));
	client.request(web::http::methods::GET).then([](web::http::http_response response) {
		if (response.status_code() == web::http::status_codes::OK) {
			size_t contentLength = response.headers().content_length();
			std::wcout << U("Content Length: ") << contentLength << std::endl;
            return response.extract_json(true);
		}
	}).then([](web::json::value data) {
        std::cout << "extract_json return " << data << std::endl;
	}).wait();
}

void test0() {
	web::http::client::http_client client(U("http://10.0.2.15:9080/cpprestsdk-client"));
	client.request(web::http::methods::GET).then([](web::http::http_response response) {
		if (response.status_code() == web::http::status_codes::OK) {
			size_t contentLength = response.headers().content_length();
			std::wcout << U("Content Length: ") << contentLength << std::endl;
            return response.extract_json(true);
		}
	}).then([](web::json::value data) {
        std::cout << "extract_json return " << data << std::endl;
	}).wait();
    std::cout << __func__ << " done " << std::endl;
}

void test1(const std::string& uristr) {
    std::cout << __func__ << " start " << std::endl;
    concurrency::streams::stringstreambuf buffer;
	web::http::client::http_client client(utility::conversions::to_string_t(uristr));
	client.request(web::http::methods::GET).then([&buffer](web::http::http_response response) {
		if (response.status_code() == web::http::status_codes::OK) {
			size_t contentLength = response.headers().content_length();
			std::wcout << U("Content Length: ") << contentLength << std::endl;  usleep(1000);
            return response.body().read_to_end(buffer);  
		}
        return pplx::task_from_result(size_t());
	}) 
    .then([](size_t data) {
        std::cout << "extract_string return " << data << std::endl;
	}).wait();
}

void test2(const std::string& uristr) {
    std::cout << __func__ << " start " << std::endl;
    concurrency::streams::stringstreambuf buffer;
	web::http::client::http_client client(utility::conversions::to_string_t(uristr));
	client.request(web::http::methods::GET).then([&buffer](web::http::http_response response) {
		if (response.status_code() == web::http::status_codes::OK) {
			size_t contentLength = response.headers().content_length();
			std::wcout << U("Content Length: ") << contentLength << std::endl;  usleep(1000);
            bool 	ignore_content_type = true;
            return response.extract_json(ignore_content_type);
		}
        return pplx::task_from_result(web::json::value());
	}) 
    .then([](web::json::value data) {
        std::cout << "extract_json return " << data << std::endl;
	})
    .wait();
}

void test_extract_string(const std::string& uristr) {
    std::cout << __func__ << " start " << std::endl;
    concurrency::streams::stringstreambuf buffer;
	web::http::client::http_client client(utility::conversions::to_string_t(uristr));
	client.request(web::http::methods::GET).then([&buffer](web::http::http_response response) {
		if (response.status_code() == web::http::status_codes::OK) {
			size_t contentLength = response.headers().content_length();
			std::wcout << U("Content Length: ") << contentLength << std::endl;  usleep(1000);
            bool 	ignore_content_type = true;
            return response.extract_string(ignore_content_type); // pplx::task<utility::string_t>
		}
        return pplx::task_from_result(utility::string_t());// pplx::task<utility::string_t>;
	}) 
    .then([](utility::string_t data) {
			std::cout << "extract_string return " << data << std::endl;
	})    
    .wait();
}

void test_extract_json(const std::string& uristr) {
    std::cout << __func__ << " start " << std::endl;
    concurrency::streams::stringstreambuf buffer;
	web::http::client::http_client client(utility::conversions::to_string_t(uristr));
	client.request(web::http::methods::GET).then([&buffer](web::http::http_response response) {
		if (response.status_code() == web::http::status_codes::OK) {
			size_t contentLength = response.headers().content_length();
			std::wcout << U("Content Length: ") << contentLength << std::endl;  usleep(1000);
            bool 	ignore_content_type = true;
            return response.extract_json(ignore_content_type); // pplx::task<json::value>
		}
        return pplx::task_from_result(web::json::value());
	}) 
    .then([](web::json::value data) {
			std::cout << "extract_json return " << data << std::endl;
	})
    .wait();
}

void test_read_to_end(const std::string& uristr) {
    std::cout << __func__ << " start " << std::endl;
    concurrency::streams::stringstreambuf buffer;
	web::http::client::http_client client(utility::conversions::to_string_t(uristr));
	client.request(web::http::methods::GET).then([&buffer](web::http::http_response response) {
		if (response.status_code() == web::http::status_codes::OK) {
			size_t contentLength = response.headers().content_length();
			std::wcout << U("Content Length: ") << contentLength << std::endl;  usleep(1000);
            bool 	ignore_content_type = true;
            return response.body().read_to_end(buffer); // pplx::task<size_t> 
		}
        return pplx::task_from_result(size_t());
	}) 
    .then([&buffer](size_t data) {
        std::string& str = buffer.collection();
        std::cout << __func__ << " read " << str.length() << " characters, " << str << std::endl;
        std::cout << "read_to_end return " << data << std::endl;
	})
    .wait();
}

bool try_sync(const std::string& url )
{
    std::cout << __func__ << " start " << url << std::endl;
    try
    {
        web::http::client::http_client client(web::uri(utility::conversions::to_string_t(url)));
        web::http::http_response response = client.request(web::http::methods::GET).get();
        concurrency::streams::stringstreambuf buffer;
        response.body().read_to_end(buffer).get();
        std::string& str = buffer.collection();
        std::cout << str << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << __func__ << " Exception: " << e.what() << std::endl;
        return false;
    }
    return true;
}

template<typename F, class ...Args1>                                                                            
void checkperf(const std::string& prompt, F& func, Args1... args) {                                                    
    auto t1 = std::chrono::high_resolution_clock::now();                                                        
    func(args...) ;                                                                                             
    auto t2 = std::chrono::high_resolution_clock::now();                                                        
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);                               
    std::chrono::duration<double, std::milli> fp_ms = t2 - t1;                                                  
    std::cout << prompt << " took " << fp_ms.count() << " ms, or " << int_ms.count() << " whole milliseconds\n";
}

int main(int argc, char* argv[]) {
    std::string urlstr = "http://10.0.2.15:9080/cpprestsdk-client";
    	boost::program_options::variables_map vm;
    boost::program_options::options_description desc("Allowed options");
	desc.add_options()("help,h", "produce help message")
    ("urlstr", boost::program_options::value<decltype(urlstr)>(&urlstr)->default_value(urlstr), "URL to get")
	;
	boost::program_options::store(
			boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);
	if(vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	} 
    // test("http://10.0.2.15:9080/cpprestsdk-client");  return 0;
    // test0();  return 0;
    // test2("http://10.0.2.15:9080/cpprestsdk-client");
    // test_extract_string("http://10.0.2.15:9080/cpprestsdk-client"); return 0;
    // test_extract_json("http://10.0.2.15:9080/cpprestsdk-client"); return 0;
    // test_read_to_end(urlstr); //return 0;
    checkperf("try_sync", try_sync, urlstr); //return 0;
    dl_iterate_phdr(dl_iterate_phdr_callback, NULL);
    std::cout << "Built with g++ "<< __GNUC__<<"." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << " "
                << argv[0] << " pid " << getpid() << " done\n";
	return 0;
}