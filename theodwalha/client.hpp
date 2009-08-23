#pragma once

#include <string>
#include <ail/file.hpp>
#include <ail/types.hpp>
#include <boost/asio.hpp>
#include <theodwalha/temporary.hpp>
#include <theodwalha/request.hpp>

struct http_server_client
{
	boost::asio::ip::tcp::socket socket;
	char * read_buffer;
	std::string extended_buffer;
	std::string temporary_file_name;
	temporary_file_manager & file_manager;
	ail::file temporary_file;
	std::size_t bytes_read;
	uword keep_alive_counter;

	bool got_header;
	http_request current_request;

	http_server_client(boost::asio::io_service & io_service, temporary_file_manager & file_manager);
	~http_server_client();

	void start();
	void initialise();
	void read();
	void terminate();
	void write(std::string const & data);

	void read_event(boost::system::error_code const & error, std::size_t bytes_in_buffer);
	void write_event(boost::system::error_code const & error, char * write_buffer);
};