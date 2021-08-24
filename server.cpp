#include <iostream>
#include <chrono>
#include <iomanip>
#include <asio.hpp>

using std::chrono::system_clock;
using asio::ip::tcp;
using namespace std::placeholders;

constexpr unsigned short kport = 18;

class Message {
public:
	uint8_t protocol_revision_;
	std::vector<uint8_t> recipent_		{ '\0' };
	std::vector<uint8_t> recip_term_	{ '\0' };
	std::vector<uint8_t> message_		{ '\0' };
	std::vector<uint8_t> sender_		{ '\0' };
	std::vector<uint8_t> sender_term_	{ '\0' };
	std::vector<uint8_t> cookie_		{ '\0' };
	std::vector<uint8_t> signature_		{ '\0' };
private:
	void GenerateCookie(const std::string format = "%y%m%d%H%M%S")
	{
		time_t time_since_epoch = system_clock::to_time_t(system_clock::now());
		std::ostringstream stream;
		stream << std::put_time(std::localtime(&time_since_epoch), format.c_str());
		std::string stream_buffer = stream.str();
		cookie_ = std::vector<uint8_t>(stream_buffer.cbegin(), stream_buffer.cend() + 1);
	}
};

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
		std::vector<uint8_t> buffer(511);
		//,,The total length of the message shall be less than 512 octets."
		size_t received_number = socket_.receive(asio::buffer(buffer));
		buffer.resize(received_number);

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
			//We have received client's request and we send daytime back.
			connection->Start();
		}
		//Then we are ready to accept another client.
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
