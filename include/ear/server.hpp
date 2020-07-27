#ifndef EAR_SERVER_HPP
#define EAR_SERVER_HPP

#include "jsonrpccxx/server.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>

#include <atomic>

jsonrpccxx::JsonRpc2Server rpcServer;

namespace ear {

class WebsocketServer {
private:
	std::atomic<bool> _stop{false};

	void _handleSocket(boost::asio::ip::tcp::socket& socket) {
		try
	    {
	        // Construct the stream by moving in the socket
	        boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws{std::move(socket)};

	        // Set a decorator to change the Server of the handshake
	        /*ws.set_option(boost::beast::websocket::stream_base::decorator(
	            [](boost::beast::websocket::response_type& res)
	            {
	                res.set(boost::beast::http::field::server,
	                    std::string(BOOST_BEAST_VERSION_STRING) +
	                        " websocket-server-sync");
	            }));*/

	        // Accept the websocket handshake
	        ws.accept();

	        while (!_stop) {
	            // This buffer will hold the incoming message
	            boost::beast::flat_buffer buffer;

	            // Read a message
	            ws.read(buffer);

				const std::string request = boost::beast::buffers_to_string(buffer.data());

				std::cout << "received message: " << request << std::endl;

				const std::string response = rpcServer.HandleRequest(request);
				boost::beast::net::const_buffer responseBuffer(response.data(), response.size());

	            // Echo the message back
	            ws.text(true);
	            ws.write(responseBuffer);
	        }
	    }
	    catch(boost::beast::system_error const& se) {
	        // This indicates that the session was closed
	        if(se.code() != boost::beast::websocket::error::closed)
	            std::cerr << "Error: " << se.code().message() << std::endl;
	    }
	    catch(std::exception const& e)
	    {
	        std::cerr << "Error: " << e.what() << std::endl;
	    }
	}

public:
	/**
	 * Start Server
	 */
	void run(unsigned short port=7777) {
		auto const address = boost::beast::net::ip::make_address("0.0.0.0");
        //auto const port = static_cast<unsigned short>(port));

        // The io_context is required for all I/O
        boost::beast::net::io_context ioc{1};

        // The acceptor receives incoming connections
        boost::asio::ip::tcp::acceptor acceptor{ioc, {address, port}};
        while (!_stop)
        {
            // This will receive the new connection
            boost::asio::ip::tcp::socket socket{ioc};

            // Block until we get a connection
            acceptor.accept(socket);

            // Launch the session, transferring ownership of the socket
            std::thread{std::bind(&WebsocketServer::_handleSocket, this,
                std::move(socket))}.detach();
        }
	}
};

class RpcServer {
	boost::beast::net::io_context& _io;
	boost::asio::ip::tcp::endpoint _endpoint;
	boost::asio::ip::tcp::acceptor _acceptor;

	void _doAccept() {
		std::cout << "doAccept" << std::endl;
		_acceptor.async_accept(boost::beast::net::make_strand(_io),
			boost::beast::bind_front_handler(&RpcServer::_onAccept, this));
	}

	void _onAccept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket) {
		if (ec) {
			std::cerr << "onAccept err=" << ec.message() << std::endl;
		} else {
			std::cout << "onAccept success" << std::endl;
		}

		_doAccept();
	}

public:
	RpcServer(boost::beast::net::io_context& io, const boost::asio::ip::tcp::endpoint& endpoint)
		: _io(io)
		, _endpoint(endpoint)
		, _acceptor(boost::beast::net::make_strand(io))
	{
		boost::beast::error_code ec;

// Open the acceptor
_acceptor.open(endpoint.protocol(), ec);
if(ec)
{
	std::cerr << "failed to open acceptor: " << ec.message() << std::endl;
	return;
}

// Allow address reuse
_acceptor.set_option(boost::beast::net::socket_base::reuse_address(true), ec);
if(ec)
{
	std::cerr << "failed to set socket reuse: " << ec.message() << std::endl;
	return;
}

// Bind to the server address
_acceptor.bind(endpoint, ec);
if(ec)
{
	std::cerr << "Failed to bind endpoint: " << ec.message() << std::endl;
	return;
}

// Start listening for connections
_acceptor.listen(
	boost::beast::net::socket_base::max_listen_connections, ec);
if(ec)
{
	std::cerr << "Failed to listen to acceptor: " << ec.message() << std::endl;
	return;
}
	}

	void run() {
		boost::beast::net::dispatch(_acceptor.get_executor(), boost::beast::bind_front_handler(&RpcServer::_doAccept, this));
	}
};

} // ear namespace

#endif
