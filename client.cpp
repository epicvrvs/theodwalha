#include <boost/bind.hpp>
#include <iostream>
#include <cstdlib>
#include "client.hpp"
#include "configuration.hpp"
#include "temporary.hpp"

http_server_client::http_server_client(boost::asio::io_service & io_service, temporary_file_manager & file_manager):
	socket(io_service),
	file_manager(file_manager),
	keep_alive_counter(keep_alive_max)
{
	read_buffer = new char[read_buffer_size];
}

http_server_client::~http_server_client()
{
	delete read_buffer;
}

void http_server_client::start()
{
	initialise();
	read();
}

void http_server_client::initialise()
{
	bytes_read = 0;
	got_header = false;
}

void http_server_client::read()
{
	socket.async_read_some(boost::asio::buffer(read_buffer, read_buffer_size), boost::bind(&http_server_client::read_event, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void http_server_client::terminate()
{
	std::cout << "Terminating" << std::endl;
	if(!temporary_file_name.empty())
	{
		temporary_file.close();
		file_manager.release(temporary_file_name);
	}
	delete this;
}

void http_server_client::read_event(boost::system::error_code const & error, std::size_t bytes_in_buffer)
{
	if(!error)
	{
		std::cout << "Read " << bytes_in_buffer << " bytes:" << std::endl;
		bytes_read += bytes_in_buffer;

		if(bytes_read > maximal_request_size)
		{
			std::cout << "Request too large" << std::endl;
			terminate();
			return;
		}

		std::string new_data(read_buffer, bytes_in_buffer);

		if(temporary_file_name.empty())
		{
			extended_buffer += new_data;

			std::cout << extended_buffer << std::endl;

			if(!got_header)
			{
				std::string error_message;
				current_request = http_request();
				process_header_result::type result = process_header(extended_buffer, current_request, error_message);
				switch(result)
				{
					case process_header_result::no_delimiter:
						std::cout << "No delimiter yet" << std::endl;
						break;

					case process_header_result::error:
						std::cout << error_message << std::endl;
						terminate();
						return;

					case process_header_result::success:
						got_header = true;
						std::cout << "Retrieved the full HTTP header" << std::endl;
						break;
				}
			}

			if(extended_buffer.size() > maximal_extended_buffer_size)
			{
				if(!got_header)
				{
					std::cout << "A client exceeded the maximal extended buffer size without delivering the HTTP header" << std::endl;
					terminate();
					return;
				}

				temporary_file_name = file_manager.generate_name();
				if(!temporary_file.open_create(temporary_file_name))
				{
					std::cout << "Failed to open temporary file " << temporary_file_name << std::endl;
					exit(1);
				}

				temporary_file.write(extended_buffer);
				extended_buffer.clear();

				std::cout << "Using temporary file name " << temporary_file_name << std::endl;
			}
		}
		else
			temporary_file.write(new_data);

		if(got_header)
		{
			std::size_t expected_byte_count = current_request.header_size + current_request.content_length;
			std::cout << "Expected byte count: " << current_request.header_size << " + " << current_request.content_length << " = " << expected_byte_count << std::endl;
			if(bytes_read > expected_byte_count)
			{
				std::cout << "Received too many bytes from a client" << std::endl;
				terminate();
				return;
			}
			else if(bytes_read == expected_byte_count)
			{
				std::cout << "Ready to serve data for " << current_request.path << std::endl;
			}
		}

		read();
	}
	else
		terminate();
}