#pragma once

#include <ail/types.hpp>
#include <boost/asio.hpp>
#include <theodwalha/client.hpp>
#include <theodwalha/temporary.hpp>
#include <theodwalha/module_manager.hpp>

class http_server
{
public:

	http_server(boost::asio::io_service & io_service, std::string const & temporary_file_directory);

	bool launch_server(ushort new_port);

private:

	bool running;
	ushort port;

	boost::asio::io_service & io_service;
	boost::asio::ip::tcp::acceptor acceptor;

	temporary_file_manager file_manager;
	module_manager modules;

	void accept();

	void accept_event(boost::system::error_code const & error, http_server_client * new_client);
};
