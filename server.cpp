#include <iostream>
#include <asio.hpp>

using asio::ip::tcp;
using namespace std::placeholders;

constexpr unsigned short kport = 18;

class TcpConnection
	: public std::enable_shared_from_this<TcpConnection> 
{
private:
	std::string message_;
	tcp::socket socket_;

	TcpConnection(asio::io_context& io_context) : socket_(io_context) 
	{
	}

	void HandleWrite(const std::error_code& error_code, size_t bytes_transferred)
	{
	}

public:
	static std::shared_ptr<TcpConnection> Create(asio::io_context& io_context)
	{
		return std::shared_ptr<TcpConnection>(new TcpConnection(io_context));
	}

	tcp::socket& Socket()
	{
		return socket_;
	}

	void Start()
	{
		message_ = "Successfully connected!";
		asio::async_write(socket_, asio::buffer(message_), 
			std::bind(&TcpConnection::HandleWrite, this, _1, _2));
	}
};

class TcpServer 
{
private:
	asio::io_context& io_context_;
	asio::ip::tcp::acceptor acceptor_;
	void StartAccept()
	{
		std::shared_ptr<TcpConnection> connection = TcpConnection::Create(io_context_);
		acceptor_.async_accept(connection->Socket(),
			std::bind(&TcpServer::HandleAccept, this, connection, _1));
	}

	void HandleAccept(std::shared_ptr<TcpConnection> connection,
		const std::error_code& error_code)
	{
		if (!error_code) {
			connection->Start();
		}
		StartAccept();
	}

public:
	TcpServer(asio::io_context& io_context)
		: io_context_(io_context),
		  acceptor_(io_context, tcp::endpoint(tcp::v4(), kport))
	{
		StartAccept();
	}
};

int main()
{
	try {
		asio::io_context io_context;
		TcpServer server(io_context);
		io_context.run();
	}
	catch (std::exception& exception) {
		std::cerr << "Error has occured: " << exception.what();
	}
}
