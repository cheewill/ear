#ifndef EAR_SERVER_HPP
#define EAR_SERVER_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>

#include <atomic>

namespace ear {

void
do_session(boost::asio::ip::tcp::socket& socket)
{
    try
    {
        // Construct the stream by moving in the socket
        boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws{std::move(socket)};

        // Set a decorator to change the Server of the handshake
        ws.set_option(boost::beast::websocket::stream_base::decorator(
            [](boost::beast::websocket::response_type& res)
            {
                res.set(boost::beast::http::field::server,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-server-sync");
            }));

        // Accept the websocket handshake
        ws.accept();

        for(;;)
        {
            // This buffer will hold the incoming message
            boost::beast::flat_buffer buffer;

            // Read a message
            ws.read(buffer);

			std::cout << "received message: " << boost::beast::buffers_to_string(buffer.data()) << std::endl;

            // Echo the message back
            ws.text(ws.got_text());
            ws.write(buffer.data());
        }
    }
    catch(boost::beast::system_error const& se)
    {
        // This indicates that the session was closed
        if(se.code() != boost::beast::websocket::error::closed)
            std::cerr << "Error: " << se.code().message() << std::endl;
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

class WebsocketServer {
private:
	std::atomic<bool> _stop{false};

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
            std::thread{std::bind(
                &do_session,
                std::move(socket))}.detach();
        }
	}
};

} // ear namespace

#endif
