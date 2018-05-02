
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <cctype>
#include <clocale>
#include <iostream>
#include <string>
#include <utility>
#include <memory>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "cassandra.h"

using namespace boost::system;
namespace filesys = boost::filesystem;

void print_error(CassFuture* future)
{
    const char* message = nullptr;
    size_t message_length{};
    cass_future_error_message(future, &message, &message_length);
    fprintf(stderr, "Error: %.*s\n", (int) message_length, message);
}

CassCluster* create_cluster(const char* hosts)
{
    CassCluster* cluster = cass_cluster_new();
    cass_cluster_set_contact_points(cluster, hosts);
    return cluster;
}

CassError connect_session(CassSession* session, const CassCluster* cluster)
{
    CassError rc = CASS_OK;
    CassFuture* future = cass_session_connect(session, cluster);
    std::unique_ptr<CassFuture, decltype(&cass_future_free)> future_inst{
        future, cass_future_free};

    cass_future_wait(future);
    rc = cass_future_error_code(future);
    if (rc != CASS_OK)
    {
        print_error(future);
    }
    return rc;
}

CassError execute_query(CassSession* session, const char* query)
{
    CassError rc = CASS_OK;
    CassStatement* statement = cass_statement_new(query, 0);
    std::unique_ptr<CassStatement, decltype(&cass_statement_free)> stmt_inst{statement, cass_statement_free};
    CassFuture* future = cass_session_execute(session, statement);
    std::unique_ptr<CassFuture, decltype(&cass_future_free)> fut_inst{future, cass_future_free};

    cass_future_wait(future);

    rc = cass_future_error_code(future);
    if (rc != CASS_OK)
    {
        print_error(future);
    }

    return rc;
}

const CassPrepared* prepare_insert_into_batch(CassSession* session, const char* query,
                                        CassError& rc)
{
    const CassPrepared* prepared = nullptr;
    rc = CASS_OK;

    std::unique_ptr<CassFuture, decltype(&cass_future_free)> future_instance{
        cass_session_prepare(session, query), cass_future_free};
    CassFuture* future = future_instance.get();
    cass_future_wait(future);

    rc = cass_future_error_code(future);
    if (rc != CASS_OK)
    {
        print_error(future);
    }
    else
    {
        prepared = cass_future_get_prepared(future);
    }

    return prepared;
}

struct fileinfo
{
	std::string filepath;
	std::size_t filesize;
	std::shared_ptr<unsigned char> filecontent;
};

CassError insert_one_file(CassSession* session, const fileinfo& onefile)
{
    const char* query = "INSERT INTO examples.pairs (key, value) VALUES (?, ?);";

    CassStatement* statement = cass_statement_new(query, 2);
    std::unique_ptr<CassStatement, decltype(&cass_statement_free)> stmt_inst{
        statement, cass_statement_free};

    cass_statement_bind_string(statement, 0, onefile.filepath.c_str());
    cass_statement_bind_bytes(statement, 1, onefile.filecontent.get(), onefile.filesize);

    CassFuture* future = cass_session_execute(session, statement);
    std::unique_ptr<CassFuture, decltype(&cass_future_free)> future_inst{
        future, cass_future_free};

    cass_future_wait(future);

    auto rc = cass_future_error_code(future);
    if (rc != CASS_OK)
    {
    	std::cerr << onefile.filepath << "\tsize: " << onefile.filesize << std::endl;
    	std::cerr << "check cassandra.yaml, increase commitlog_segment_size_in_mb: 512\n";
        print_error(future);
    }

    return rc;
}

CassError insert_into_batch_with_prepared(CassSession* session,
                                          const CassPrepared* prepared,
										  const std::vector<fileinfo>& files)
{
    CassError rc = CASS_OK;
    CassBatch* batch = cass_batch_new(CASS_BATCH_TYPE_LOGGED);
    std::unique_ptr<CassBatch, decltype(&cass_batch_free)> batch_inst(batch, cass_batch_free);
    std::size_t rows = 0;

    for (const auto& file: files)
    {
        CassStatement* statement = cass_prepared_bind(prepared);
        cass_statement_bind_string(statement, 0, file.filepath.c_str());
        cass_statement_bind_bytes(statement, 1, file.filecontent.get(), file.filesize);
        cass_batch_add_statement(batch, statement);
        cass_statement_free(statement);
        ++rows;
        if (rows)
        {
        	rows = 0;
            CassFuture* future = cass_session_execute_batch(session, batch);
            std::unique_ptr<CassFuture, decltype(&cass_future_free)> fut_inst(future, cass_future_free);

            cass_future_wait(future);

            rc = cass_future_error_code(future);
            if (rc != CASS_OK)
            {
                print_error(future);
            }
            batch_inst.reset(cass_batch_new(CASS_BATCH_TYPE_LOGGED));
            batch = batch_inst.get();
        }
    }
    if (rows)
    {
        CassFuture* future = cass_session_execute_batch(session, batch);
        std::unique_ptr<CassFuture, decltype(&cass_future_free)> fut_inst(future, cass_future_free);

        cass_future_wait(future);

        rc = cass_future_error_code(future);
        if (rc != CASS_OK)
        {
            print_error(future);
        }

    }

    return rc;
}

int max_dump_rows = 100;
int max_col_len = 128;

CassError run_query(CassSession* session, const char* query)
{
    CassStatement* statement = cass_statement_new(query, 0);
    std::unique_ptr<CassStatement, decltype(&cass_statement_free)> stmt_inst(
        statement, cass_statement_free);

    CassFuture* result_future = cass_session_execute(session, statement);
    std::unique_ptr<CassFuture, decltype(&cass_future_free)> res_fut_inst(
        result_future, cass_future_free);
    std::size_t row_cnt = 0;

    if (cass_future_error_code(result_future) == CASS_OK)
    {
        const CassResult* result = cass_future_get_result(result_future);
        std::unique_ptr<const CassResult, decltype(&cass_result_free)> result_inst(
            result, cass_result_free);
        auto ccount = cass_result_column_count(result);

        for (size_t i = 0; i < ccount; ++i)
        {
            const char* namep;
            unsigned long int len = 9;
            auto rc = cass_result_column_name(result, i, &namep, &len);
            if (rc == CASS_OK)
                std::cout << namep << "\t";
            else
                std::cout << "column-" << i << "\t";
        }
        std::cout << "\n";

        CassIterator* iterator_row = cass_iterator_from_result(result);
        std::unique_ptr<CassIterator, decltype(&cass_iterator_free)> iter_inst(
            iterator_row, cass_iterator_free);

        while (cass_iterator_next(iterator_row))
        {
            ++row_cnt;
            bool print_to_console = row_cnt < max_dump_rows;

            const CassRow* row = cass_iterator_get_row(iterator_row);
            CassIterator* iterator_column = cass_iterator_from_row(row);
            std::unique_ptr<CassIterator, decltype(&cass_iterator_free)> iter_col_inst(
                iterator_column, cass_iterator_free);

            while (cass_iterator_next(iterator_column))
            {
                cass_int32_t i;
                cass_int64_t bi;
                cass_bool_t b;
                cass_double_t d;
                cass_float_t f;
                const char* s;
                const cass_byte_t* blob;
                size_t s_length;
                CassUuid u;
                char us[CASS_UUID_STRING_LENGTH];
                CassError rc = CASS_OK;

                const CassValue* value = cass_iterator_get_column(iterator_column);
                CassValueType type = cass_value_type(value);

                if (!print_to_console)
                    continue; // don't dump, but iterate all rows

                switch (type)
                {
                case CASS_VALUE_TYPE_INT:
                    cass_value_get_int32(value, &i);
                    std::cout << i << "\t";
                    break;

                case CASS_VALUE_TYPE_BIGINT:
                    cass_value_get_int64(value, &bi);
                    std::cout << bi << "\t";
                    break;

                case CASS_VALUE_TYPE_BOOLEAN:
                    cass_value_get_bool(value, &b);
                    std::cout << std::boolalpha << b << "\t";
                    break;

                case CASS_VALUE_TYPE_DOUBLE:
                    cass_value_get_double(value, &d);
                    std::cout << d << "\t";
                    break;

                case CASS_VALUE_TYPE_FLOAT:
                    cass_value_get_float(value, &f);
                    std::cout << f << "\t";
                    break;
                case CASS_VALUE_TYPE_TEXT:
                case CASS_VALUE_TYPE_ASCII:
                case CASS_VALUE_TYPE_VARCHAR:
                    rc = cass_value_get_string(value, &s, &s_length);
                    if (rc != CASS_OK)
                    {
                        std::cerr << "cass_value_get_string error " << rc << "\t";
                    }
                    else
                    {
                        bool isprint = !std::any_of(
                            s, s + s_length, [](char c) { return std::isprint(c) == 0; });

                        if (isprint)
                        {
                            std::cout
                                << std::string(s, std::min(max_col_len, (int) s_length))
                                << "\t";
                        }
                        else
                            std::cout << "binary length " << s_length << "\t";
                    }
                    break;
                case CASS_VALUE_TYPE_BLOB:
                    rc = cass_value_get_bytes(value, &blob, &s_length);
                    if (rc != CASS_OK)
                    {
                        std::cerr << "cass_value_get_bytes error " << rc << "\t";
                    }
                    else
                    {
                        bool can_print = !std::any_of(
                            blob,
                            blob + std::min(static_cast<decltype(s_length)>(8), s_length),
                            [](char c) { return std::isprint(c) == 0; });

                        if (can_print)
                        {
                            std::cout
                                << std::string((const char*) blob,
                                               std::min(max_col_len, (int) s_length))
                                << "\t";
                        }
                        else
                            std::cout << "binary length " << s_length << "\t";
                    }
                    break;
                case CASS_VALUE_TYPE_UUID:
                    cass_value_get_uuid(value, &u);
                    cass_uuid_string(u, us);
                    break;

                default:
                    if (cass_value_is_null(value))
                    {
                        std::cout << "null\t";
                    }
                    else
                    {
                        std::cout << "unhandled type " << type << "\t";
                    }
                    break;
                }
            }
            if (print_to_console)
                std::cout << "\n";
        }
        std::cout << "Read " << row_cnt << " rows\n";
    }
    else
    {
        const char* message = nullptr;
        size_t message_length = 0;
        cass_future_error_message(result_future, &message, &message_length);
        fprintf(stderr, "Unable to run query: '%.*s'\n", (int) message_length, message);
    }
}

std::vector<std::string> get_all_file_path(
    const std::string& target_dir, const std::vector<std::string> ignore_list = {})
{
    std::vector<std::string> file_path_list;

    try
    {
        if (filesys::exists(target_dir) && filesys::is_directory(target_dir))
        {
            filesys::recursive_directory_iterator iter(target_dir);
            filesys::recursive_directory_iterator end;

            while (iter != end)
            {
                if (filesys::is_directory(iter->path()) &&
                    (std::find(ignore_list.begin(), ignore_list.end(),
                               iter->path().filename()) != ignore_list.end()))
                {
                    iter.no_push();
                }
                else
                {
                    if (!filesys::is_directory(iter->path()))
                        file_path_list.push_back(iter->path().string());
                }

                error_code ec{};

                iter.increment(ec);
                if (ec)
                {
                    std::cerr << "Error While Accessing : " << iter->path().string()
                              << " :: " << ec.message() << '\n';
                }
            }
        }
    }
    catch (std::system_error& e)
    {
        std::cerr << "Exception : " << e.what() << std::endl;
    }
    return file_path_list;
}

int main(int argc, char* argv[])
{
    std::cout << "tested with apache-cassandra-3.11.2-bin.tar.gz, need to change "
                 "cassandra.yaml like prepared_statements_cache_size_mb "
                 "commitlog_segment_size_in_mb\n";
    std::cout << "I'm using https://github.com/datastax/cpp-driver\n";

    std::setlocale(LC_ALL, "en_GB.iso88591");
    std::unique_ptr<CassSession, decltype(&cass_session_free)> session_instance(
        cass_session_new(), cass_session_free);
    CassSession* session = session_instance.get();

    std::string server_address = "localhost";
    std::size_t max_bytes_to_write = 1024*1024*512;
    int enable_batch = 0;
    bool drop_table = true;
    std::string source_dir = getenv("HOME");
    {
        char current_dir[PATH_MAX];
        if (getcwd(current_dir, sizeof(current_dir)) == nullptr)
        {
        	std::cerr << "getcwd error\n";
        }
        else
        	source_dir = current_dir;
    }

    boost::program_options::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message")
		("server",
        boost::program_options::value<decltype(server_address)>(&server_address)
            ->default_value(server_address), "cassandra Server address")
		("max_dump_rows",
		boost::program_options::value<decltype(max_dump_rows)>(&max_dump_rows)
			->default_value(max_dump_rows), "Max number of rows to dump to console")
		("enable_batch",
		boost::program_options::value<decltype(enable_batch)>(&enable_batch)
			->default_value(enable_batch), "Max number of rows to dump to console")
		("max_bytes_to_write",
		boost::program_options::value<decltype(max_bytes_to_write)>(&max_bytes_to_write)
			->default_value(max_bytes_to_write), "Max number of bytes to write into cassandra")
		("source_dir",
		boost::program_options::value<decltype(source_dir)>(&source_dir)
			->default_value(source_dir), "Path of directory to store into cassandra")
		("max_col_len",
		boost::program_options::value<decltype(max_col_len)>(&max_col_len)
			->default_value(max_col_len), "max number of chars to display per column")
		("drop_table",
		boost::program_options::value<decltype(drop_table)>(&drop_table)
			->default_value(drop_table), "specify if table should be dropped before create")
		;

    boost::program_options::variables_map vm;
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);
    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 0;
    }

    std::unique_ptr<CassCluster, decltype(&cass_cluster_free)> cluster(
        create_cluster(server_address.c_str()), cass_cluster_free);
    if (connect_session(session, cluster.get()) != CASS_OK)
    {
        std::cerr << argv[0] << " failed to connect to " << server_address << std::endl;
        return -1;
    }
    cass_cluster_set_request_timeout(cluster.get(), 5000);

    if (drop_table)
    	execute_query(session, "DROP TABLE IF EXISTS examples.pairs;");

    execute_query(session, "CREATE KEYSPACE IF NOT EXISTS examples WITH replication = { \
                           'class': 'SimpleStrategy', 'replication_factor': '1' };");

    execute_query(session, "CREATE TABLE IF NOT EXISTS examples.pairs (key text, \
                                              value blob, \
                                              PRIMARY KEY (key));");
    run_query(session, "SELECT release_version FROM system.local");

    CassError rc = CASS_ERROR_LAST_ENTRY;
    const CassPrepared* prepared = prepare_insert_into_batch(
        session, "INSERT INTO examples.pairs (key, value) VALUES (?, ?)",
        rc);
    std::unique_ptr<const CassPrepared, decltype(&cass_prepared_free)> prep_inst(
        prepared, cass_prepared_free);

    if (rc != CASS_OK)
    {
    	std::cerr << "prepare_insert_into_batch error code " << rc << std::endl;
    	return rc;
    }

    std::vector<fileinfo> files;
    std::vector<std::string> filelist = get_all_file_path(source_dir);
    std::size_t total_bytes = 0;

    for (const auto& onefile: filelist)
    {
    	std::ifstream in(onefile.c_str(), std::ifstream::ate | std::ifstream::binary);
    	std::size_t filesize = in.tellg();
    	if (!in)
    	{
    		std::cerr << "can't read " << onefile << std::endl;
    		continue;
    	}

    	if (filesize + total_bytes > max_bytes_to_write)
    	{
    		break;
    	}

    	total_bytes += filesize;
    	in.seekg (0, in.beg);
    	std::shared_ptr<unsigned char> buf{new unsigned char[filesize], std::default_delete<unsigned char[]>()};
    	in.read( (char*)buf.get(), filesize);
    	files.emplace_back(fileinfo{onefile, filesize, buf});
    }

    {
        auto t1 = std::chrono::high_resolution_clock::now();

        if (enable_batch)
        	insert_into_batch_with_prepared(session, prepared, files);
        else
        for (auto& onefile: files)
        	insert_one_file(session, onefile);

        auto t2 = std::chrono::high_resolution_clock::now();
        auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

        std::cout << "it took " << int_ms << " milliseconds to insert " << filelist.size()
        		<< " files, " << total_bytes << " bytes\n";
    }

    {
        auto t1 = std::chrono::high_resolution_clock::now();
        run_query(session, "SELECT key, value FROM examples.pairs LIMIT 3");
        auto t2 = std::chrono::high_resolution_clock::now();
        auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

        std::cout << "it took " << int_ms << " milliseconds to read examples.pairs\n";
    }

    std::unique_ptr<CassFuture, decltype(&cass_future_free)> close_future(
        cass_session_close(session), cass_future_free);

    cass_future_wait(close_future.get());

    return 0;
}
